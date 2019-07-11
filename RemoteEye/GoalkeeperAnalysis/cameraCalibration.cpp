#include <cameraCalibration.h>

using namespace std;

static void read(const FileNode& node, Settings& x, const Settings& default_value = Settings())
{
	if (node.empty())
		x = default_value;
	else
		x.read(node);
}


CameraCalibration::CameraCalibration() {}

void CameraCalibration::start()
{
	help();
	Settings s;
	const string inputSettingsFile = "CameraCalibration/calibrationConfig.xml";
	FileStorage fs(inputSettingsFile, FileStorage::READ); // Read the settings
	if (!fs.isOpened())
	{
		cout << "Could not open the configuration file: \"" << inputSettingsFile << "\"" << endl;
	}
	fs["Settings"] >> s;
	fs.release(); // close Settings file

	if (!s.goodInput)
	{
		cout << "Invalid input detected. Application stopping. " << endl;
	}


	int mode = s.inputType == Settings::IMAGE_LIST ? CAPTURING : DETECTION;
	clock_t prevTimestamp = 0;
	const Scalar RED(0, 0, 255), GREEN(0, 255, 0);
	const char ESC_KEY = 27;

	for (int i = 0;; ++i)
	{
		Mat view;
		bool blinkOutput = false;

		view = s.nextImage();

		//-----  If no more image, or got enough, then stop calibration and show result -------------
		if (mode == CAPTURING && imagePoints.size() >= (unsigned)s.nrFrames)
		{
			if (runCalibrationAndSave(s, imageSize, cameraMatrix, distCoeffs, imagePoints))
				mode = CALIBRATED;
			else
				mode = DETECTION;
		}
		if (view.empty()) // If no more images then run calibration, save and stop loop.
		{
			if (imagePoints.size() > 0)
				runCalibrationAndSave(s, imageSize, cameraMatrix, distCoeffs, imagePoints);
			break;
		}


		imageSize = view.size(); // Format input image.
		if (s.flipVertical) flip(view, view, 0);

		vector<Point2f> pointBuf;

		bool found;
		switch (s.calibrationPattern) // Find feature points on the input format
		{
		case Settings::CHESSBOARD:
			found = findChessboardCorners(view, s.boardSize, pointBuf,
			                              CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
			break;
		case Settings::CIRCLES_GRID:
			found = findCirclesGrid(view, s.boardSize, pointBuf);
			break;
		case Settings::ASYMMETRIC_CIRCLES_GRID:
			found = findCirclesGrid(view, s.boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID);
			break;
		default:
			found = false;
			break;
		}

		if (found) // If done with success,
		{
			// improve the found corners' coordinate accuracy for chessboard
			if (s.calibrationPattern == Settings::CHESSBOARD)
			{
				Mat viewGray;
				cvtColor(view, viewGray, COLOR_BGR2GRAY);
				cornerSubPix(viewGray, pointBuf, Size(11, 11),
				             Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
			}

			if (mode == CAPTURING && // For camera only take new samples after delay time
				(!s.inputCapture.isOpened() || clock() - prevTimestamp > s.delay * 1e-3 * CLOCKS_PER_SEC))
			{
				imagePoints.push_back(pointBuf);
				prevTimestamp = clock();
				blinkOutput = s.inputCapture.isOpened();
			}

			// Draw the corners.
			drawChessboardCorners(view, s.boardSize, Mat(pointBuf), found);
		}

		//----------------------------- Output Text ------------------------------------------------
		string msg = (mode == CAPTURING) ? "100/100" : mode == CALIBRATED ? "Calibrated" : "Press 'g' to start";
		int baseLine = 0;
		Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
		Point textOrigin(view.cols - 2 * textSize.width - 10, view.rows - 2 * baseLine - 10);

		if (mode == CAPTURING)
		{
			if (s.showUndistorsed)
				msg = format("%d/%d Undist", (int)imagePoints.size(), s.nrFrames);
			else
				msg = format("%d/%d", (int)imagePoints.size(), s.nrFrames);
		}

		putText(view, msg, textOrigin, 1, 1, mode == CALIBRATED ? GREEN : RED);

		if (blinkOutput)
			bitwise_not(view, view);

		//------------------------- Video capture  output  undistorted ------------------------------
		if (mode == CALIBRATED && s.showUndistorsed)
		{
			Mat temp = view.clone();
			undistort(temp, view, cameraMatrix, distCoeffs);
		}

		//------------------------------ Show image and check for input commands -------------------
		imshow("Image View", view);
		char key = (char)waitKey(s.inputCapture.isOpened() ? 50 : s.delay);

		if (key == ESC_KEY)
			break;

		if (key == 'u' && mode == CALIBRATED)
			s.showUndistorsed = !s.showUndistorsed;

		if (s.inputCapture.isOpened() && key == 'g')
		{
			mode = CAPTURING;
			imagePoints.clear();
		}
	}

	// -----------------------Show the undistorted image for the image list ------------------------
	/*if (s.inputType == Settings::IMAGE_LIST && s.showUndistorsed)
	{
	Mat view, rview, map1, map2;
	initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
	getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
	imageSize, CV_16SC2, map1, map2);

	for (int i = 0; i < (int)s.imageList.size(); i++)
	{
	view = imread(s.imageList[i], 1);
	if (view.empty())
	continue;
	remap(view, rview, map1, map2, INTER_LINEAR);
	imshow("Image View", rview);
	char c = (char)waitKey();
	if (c == ESC_KEY || c == 'q' || c == 'Q')
	break;
	}
	}*/

	destroyWindow("Image View");

	getIntrinsicParameters();
	getExtrinsicParameters();
}

double CameraCalibration::computeReprojectionErrors(const vector<vector<Point3f>>& objectPoints,
                                                    const vector<vector<Point2f>>& imagePoints,
                                                    const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                                    const Mat& cameraMatrix, const Mat& distCoeffs,
                                                    vector<float>& perViewErrors)
{
	vector<Point2f> imagePoints2;
	int i, totalPoints = 0;
	double totalErr = 0, err;
	perViewErrors.resize(objectPoints.size());

	for (i = 0; i < (int)objectPoints.size(); ++i)
	{
		projectPoints(Mat(objectPoints[i]), rvecs[i], tvecs[i], cameraMatrix, distCoeffs, imagePoints2);
		err = norm(Mat(imagePoints[i]), Mat(imagePoints2), CV_L2);

		int n = (int)objectPoints[i].size();
		perViewErrors[i] = (float)std::sqrt(err * err / n);
		totalErr += err * err;
		totalPoints += n;
	}

	return std::sqrt(totalErr / totalPoints);
}

void CameraCalibration::calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f>& corners,
                                                 Settings::Pattern patternType /*= Settings::CHESSBOARD*/)
{
	corners.clear();

	switch (patternType)
	{
	case Settings::CHESSBOARD:
	case Settings::CIRCLES_GRID:
		for (int i = 0; i < boardSize.height; ++i)
			for (int j = 0; j < boardSize.width; ++j)
				corners.push_back(Point3f(float(j * squareSize), float(i * squareSize), 0));
		break;

	case Settings::ASYMMETRIC_CIRCLES_GRID:
		for (int i = 0; i < boardSize.height; i++)
			for (int j = 0; j < boardSize.width; j++)
				corners.push_back(Point3f(float((2 * j + i % 2) * squareSize), float(i * squareSize), 0));
		break;
	default:
		break;
	}
}

bool CameraCalibration::runCalibration(Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                                       vector<vector<Point2f>> imagePoints, vector<Mat>& rvecs, vector<Mat>& tvecs,
                                       vector<float>& reprojErrs, double& totalAvgErr)
{
	cameraMatrix = Mat::eye(3, 3, CV_64F);
	if (s.flag & CV_CALIB_FIX_ASPECT_RATIO)
		cameraMatrix.at<double>(0, 0) = 1.0;

	distCoeffs = Mat::zeros(8, 1, CV_64F);

	vector<vector<Point3f>> objectPoints(1);
	calcBoardCornerPositions(s.boardSize, s.squareSize, objectPoints[0], s.calibrationPattern);

	objectPoints.resize(imagePoints.size(), objectPoints[0]);

	//Find intrinsic and extrinsic camera parameters
	double rms = calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix,
	                             distCoeffs, rvecs, tvecs, s.flag | CV_CALIB_FIX_K4 | CV_CALIB_FIX_K5);

	cout << "Re-projection error reported by calibrateCamera: " << rms << endl;

	bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

	totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
	                                        rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

	objectPoints_copy = objectPoints[0];
	return ok;
	return true;
}

// Print camera parameters to the output file
void CameraCalibration::saveCameraParams(Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                                         const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                         const vector<float>& reprojErrs, const vector<vector<Point2f>>& imagePoints,
                                         double totalAvgErr)
{
	FileStorage fs(s.outputFileName, FileStorage::WRITE);

	time_t tm;
	time(&tm);
	struct tm* t2 = localtime(&tm);
	char buf[1024];
	strftime(buf, sizeof(buf) - 1, "%c", t2);

	fs << "calibration_Time" << buf;

	if (!rvecs.empty() || !reprojErrs.empty())
		fs << "nrOfFrames" << (int)std::max(rvecs.size(), reprojErrs.size());
	fs << "image_Width" << imageSize.width;
	fs << "image_Height" << imageSize.height;
	fs << "board_Width" << s.boardSize.width;
	fs << "board_Height" << s.boardSize.height;
	fs << "square_Size" << s.squareSize;

	if (s.flag & CV_CALIB_FIX_ASPECT_RATIO)
		fs << "FixAspectRatio" << s.aspectRatio;

	if (s.flag)
	{
		sprintf(buf, "flags: %s%s%s%s",
		        s.flag & CV_CALIB_USE_INTRINSIC_GUESS ? " +use_intrinsic_guess" : "",
		        s.flag & CV_CALIB_FIX_ASPECT_RATIO ? " +fix_aspectRatio" : "",
		        s.flag & CV_CALIB_FIX_PRINCIPAL_POINT ? " +fix_principal_point" : "",
		        s.flag & CV_CALIB_ZERO_TANGENT_DIST ? " +zero_tangent_dist" : "");
		cvWriteComment(*fs, buf, 0);
	}

	fs << "flagValue" << s.flag;

	fs << "Camera_Matrix" << cameraMatrix;
	fs << "Distortion_Coefficients" << distCoeffs;

	fs << "Avg_Reprojection_Error" << totalAvgErr;
	if (!reprojErrs.empty())
		fs << "Per_View_Reprojection_Errors" << Mat(reprojErrs);

	if (!rvecs.empty() && !tvecs.empty())
	{
		CV_Assert(rvecs[0].type() == tvecs[0].type());
		Mat bigmat((int)rvecs.size(), 6, rvecs[0].type());
		for (int i = 0; i < (int)rvecs.size(); i++)
		{
			Mat r = bigmat(Range(i, i + 1), Range(0, 3));
			Mat t = bigmat(Range(i, i + 1), Range(3, 6));

			CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
			CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
			//*.t() is MatExpr (not Mat) so we can use assignment operator
			r = rvecs[i].t();
			t = tvecs[i].t();
		}
		cvWriteComment(*fs, "a set of 6-tuples (rotation vector + translation vector) for each view", 0);
		fs << "Extrinsic_Parameters" << bigmat;
	}

	if (!imagePoints.empty())
	{
		Mat imagePtMat((int)imagePoints.size(), (int)imagePoints[0].size(), CV_32FC2);
		for (int i = 0; i < (int)imagePoints.size(); i++)
		{
			Mat r = imagePtMat.row(i).reshape(2, imagePtMat.cols);
			Mat imgpti(imagePoints[i]);
			imgpti.copyTo(r);
		}
		fs << "Image_points" << imagePtMat;
	}
}

bool CameraCalibration::runCalibrationAndSave(Settings& s, Size imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                                              vector<vector<Point2f>> imagePoints)
{
	double totalAvgErr = 0;

	bool ok = runCalibration(s, imageSize, cameraMatrix, distCoeffs, imagePoints, rvecs, tvecs, reprojErrs, totalAvgErr);
	cout << (ok ? "Calibration succeeded" : "Calibration failed")
		<< ". avg re projection error = " << totalAvgErr;

	if (ok)
		saveCameraParams(s, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs, reprojErrs, imagePoints, totalAvgErr);
	return ok;
}

void CameraCalibration::getIntrinsicParameters()
{
	// Sensor width
	apertureWidth = 3.840;

	// Sensor height
	apertureHeight = 2.880;

	// 1 pixel is 4,8 micrometer
	pixelSizeInMM = 0.0048;

	// Sensor width in pixels
	long imageSizeWidth = apertureWidth / pixelSizeInMM;

	// Sensor height in pixels
	long imageSizeHeight = apertureHeight / pixelSizeInMM;

	// Sensor size in pixels
	imageSize = Size(imageSizeWidth, imageSizeHeight);


	double theoreticalCenter_r = apertureWidth / 2;
	double theoreticalCenter_c = apertureHeight / 2;

	// get Intrinsic parameteres
	calibrationMatrixValues(cameraMatrix, imageSize, apertureWidth, apertureHeight, fovx, fovy, focalLength,
	                        principalPoint, aspectRatio);

	// pixelPitch in mm
	pixelPitch_mm_r = theoreticalCenter_r - principalPoint.x;
	pixelPitch_mm_c = theoreticalCenter_c - principalPoint.y;

	// principal point in pixels
	principalPoint_pixels_r = principalPoint.x / pixelSizeInMM;
	principalPoint_pixels_c = principalPoint.y / pixelSizeInMM;


	qDebug() << "\n\n Calibration results: ";

	qDebug() << "Imagesensor size in mm.     =>  Width = " << apertureWidth << " Height = " << apertureHeight;
	qDebug() << "Imagesensor size in pixels. =>  Width = " << imageSize.width << " Height = " << imageSize.height;

	qDebug() << "focallength in mm: " << focalLength;

	qDebug() << "principalPoint in mm.      => x = " << principalPoint.x << "mm, y= " << principalPoint.y << " mm";
	qDebug() << "Principalpoint in pixels   => rows = " << principalPoint_pixels_r << " columns = " <<
		principalPoint_pixels_c;

	qDebug() << "Calculated pixelPitch column in mm: " << pixelPitch_mm_c;
	qDebug() << "Calculated pixelPitch roww in mm: " << pixelPitch_mm_r;
}


void CameraCalibration::getExtrinsicParameters()
{
	// Camera internals
	cv::Mat camera_matrix = (cv::Mat_<double>(3, 3) << focalLength, 0, principalPoint.x, 0, focalLength, principalPoint.y,
		0, 0, 1);
	cv::Mat dist_coeffs = distCoeffs;

	cout << "Camera Matrix " << endl << camera_matrix << endl;

	// Solve for pose
	cv::solvePnP(objectPoints_copy, imagePoints[0], camera_matrix, dist_coeffs, rotation_vector, tmp_translation_vector);

	rotationMatrix = (cv::Mat_<double>(3, 3) << rotation_vector.at<double>(0), 0, 0, 0, rotation_vector.at<double>(1), 0, 0
		, 0, rotation_vector.at<double>(2));

	cout << endl << "rotationMatrix  " << endl << rotationMatrix << endl << endl;
	std::cout << endl << "Translation Vector" << endl << tmp_translation_vector << endl << endl;

	translationVector.x = tmp_translation_vector.at<double>(0);
	translationVector.y = tmp_translation_vector.at<double>(1);
	translationVector.z = tmp_translation_vector.at<double>(2);
}
