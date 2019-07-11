#include "DetectorCode.h"

#include "IntermediateTypes.h"

#include <future>
#include <list>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QDebug>
#include "PupilDetection.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ProfilingMonitor.h"
#include "constants.h"


// temporary global for the debug flag in these methods to distinguish from the debug flags in all the other code before
// this is changed.
bool local_debug = false;

struct SortCvPoint2fByX
{
	bool operator()(const cv::Point2f& lhs, const cv::Point2f& rhs)
	{
		return lhs.x < rhs.x;
	}
};


#ifdef QT_CORE_LIB
	QDebug& debug_logger()
	{
		return qDebug();
	}
#else
	std::ostream& debug_logger()
	{
		return std::cout;
	}
#endif


std::vector<cv::Point2f> findGeometry(const std::vector<cv::Point2f>& glintCandidates, cv::Mat src)
{
	// show lines between Point2fs
	std::vector<cv::Point2f> glintCombi;

	std::list<cv::Point2f> glintList(glintCandidates.begin(), glintCandidates.end());
	unsigned char minPixelx = 1;
	unsigned char maxPixelx = 8;

	unsigned char minPixely = 0;
	unsigned char maxPixely = 5;


	// Go throuth each Point2f in list
	for (auto it = glintList.begin(); it != glintList.end(); ++it)
	{
		// find Point2f which is +- 14 Pixels on x axis					horizontal: min 7, max 14
		//				   and +- 7 Pixels on y axis
		for (auto it1 = glintList.begin(); it1 != glintList.end(); ++it1)
		{
			int x = abs((*it).x - (*it1).x);
			int y = abs((*it).y - (*it1).y);


			// found horizontal Point2f pair
//			if (x < 16 && x > 4 && y < 3 && y >= 0) // original
			if (x < 20 && x > 5 && y < 5 && y >= 0) // original
			//if (x > 5 && y < 5 && y >= 0)			// tanja not working
													//if (x < 18 && x > 3 && y < 5 && y >= 0)	// best only left side is wrong, because of double deteciton in 1 eye
													//if (x < 20 && x > 3 && y < 5 && y >= 0)	// best only left side is wrong
			{
				// Point2f i is right  => search for third Point2f to the left
				if ((*it).x >(*it1).x && (*it).y + 2 < (*it1).y)
				{
					// search for third Point2f to the left
					// where third Point2f  - 4 to -8 on x axis
					//				and   + 3 to +8 on y axis
					for (std::list<cv::Point2f>::iterator it2 = glintList.begin(); it2 != glintList.end(); ++it2)
					{
						int x = (*it).x - (*it2).x;
						int y = (*it).y - (*it2).y;


						// right Point2f to mid down Point2f
						// ABEND
						//if (x < 10 && x > 1 && y < 7 && y >= 2) // original
						//if (x > 5 && y < 5 && y >= 0) // original
						if (x < 10 && x > 5 && y < 5 && y >= 0) // original
						{
							int x = (*it2).x - (*it1).x;
							int y = (*it1).y - (*it2).y;

							// mid down Point2f to left Point2f
							// ABEND
							//if (x < 10 && x > 1 && y < 7 && y >= 2) // original
							if (x > 5 && y < 7 && y >= 2) // original
																	//if (x < 10 && x >= 1 && y < 7 && y >= 1)  // best only left side is wrong
																	//if (x < 10 && x > 1 && y < 7 && y >= 2)
							{
								if (local_debug)
								{
									line(src, (*it), (*it1), cv::Scalar(0, 0, 255), 1, 8, 0); // red		= 1. horizontal line a - b
									line(src, (*it), (*it2), cv::Scalar(255, 0, 255), 1, 8, 0); // magenta	= 2. diagonal line   a - c  
									line(src, (*it2), (*it1), cv::Scalar(0, 255, 255), 1, 8, 0); // yellow	= 3. diagonal line   c - b
								}

								std::vector<cv::Point2f> tmp;
								tmp.emplace_back((*it));
								tmp.emplace_back((*it1));
								tmp.emplace_back((*it2));

								glintCombi = tmp;
								return glintCombi;
							}
						}
					}
				}
				else // Point2f i is left => search for third Point2f to the right
				{
					// search for third Point2f to the left
					// where third Point2f  - 4 to -8 on x axis
					//				and   + 3 to +8 on y axis
					for (std::list<cv::Point2f>::iterator it2 = glintList.begin(); it2 != glintList.end(); ++it2)
					{
						int x = (*it2).x - (*it).x;
						int y = (*it2).y - (*it).y;

						// left Point2f to mid down Point2f
						// ABEND
						//if (x < 10 && x > 2 && y < 7 && y >= 3)
						//if (x < 8 && x >1 && y < 7 && y >= 2)
						//if (x < 10 && x > 1 && y < 7 && y >= 2) // original
						//if (x > 5 && y < 5 && y >= 0) // original
						if (x < 10 && x > 5 && y < 5 && y >= 0) // original
																//if (x < 10 && x >= 1 && y < 7 && y >= 1)	// best only left side is wrong
						{
							//debug_logger() << " x: " << x << " y: " << y;

							int x = (*it1).x - (*it2).x;
							int y = (*it2).y - (*it1).y;

							// mid down Point2f to up right Point2f
							// ABEND
							//if (x < 10 && x > 1 && y < 7 && y >= 2) // original
							if (x > 5 && y < 5 && y >= 0) // original
																	//if (x < 10 && x >= 1 && y < 7 && y >= 1)	// best only left side is wrong
																	//if (x < 8 && x > 1 && y < 7 && y >= 2)
							{
								if (local_debug)
								{
									line(src, (*it), (*it1), cv::Scalar(0, 0, 255), 1, 8, 0); // red		= 1. horizontal line a - b
									line(src, (*it), (*it2), cv::Scalar(255, 0, 255), 1, 8, 0); // magenta	= 2. diagonal line   a - c  
									line(src, (*it2), (*it1), cv::Scalar(0, 255, 255), 1, 8, 0); // yellow	= 3. diagonal line   c - b
								}
								std::vector<cv::Point2f> tmp;
								tmp.emplace_back((*it));
								tmp.emplace_back((*it1));
								tmp.emplace_back((*it2));

								glintCombi = tmp;
								//break;
								return glintCombi;
							}
						}
					}
				}
			}
		}

		//it = glintList.erase(it);
	}

	if (local_debug)
	{
		debug_logger() << "\n3. New combination:";
	}


	for (int i = 0; i < glintCombi.size(); i++)
	{
		if (local_debug)
		{
			debug_logger() << "Combination: " << i;
			debug_logger() << glintCombi[i].x << glintCombi[i].y;
			debug_logger() << glintCombi[i].x << glintCombi[i].y;
			debug_logger() << glintCombi[i].x << glintCombi[i].y << "\n";
		}
		// NEW
		//equalizeGlints(src, glintCombi[i]);
	}

	return glintCombi;
}

std::vector<cv::Point2f> removeFalseGlints(std::vector<cv::Point2f> contourCenters, cv::Mat thresh, cv::Mat src)
{
	const unsigned char a = 3;
	// Normal case. 
	//int maxMeanAroundGlint = 80; 

	// just to test with all 
	int maxMeanAroundGlint = 255; // 60
	std::vector<cv::Point2f> glintCandidates;

	if (local_debug) { debug_logger() << "\n2. removed false Glints:"; }


	for (int i = 0; i < contourCenters.size(); i++)
	{
		cv::Point2f center = contourCenters[i];
		cv::Point2f delta = { 3.0, 3.0 };
		cv::Rect tmp = { center - delta, center + delta };
		cv::Rect boundaries = { 0, 0, src.cols, src.rows };

		if (tmp.tl().inside(boundaries) && tmp.br().inside(boundaries))
		{
			double count =
				src.at<unsigned char>(center.y + a, center.x + a) // x+3, y +3
				+ src.at<unsigned char>(center.y + a - 1, center.x + a) // x+3, y +2
				+ src.at<unsigned char>(center.y + a - 2, center.x + a) // x+3, y +1
				+ src.at<unsigned char>(center.y + a - 3, center.x + a) // x+3, y 
				+ src.at<unsigned char>(center.y + a - 4, center.x + a) // x+3, y -1
				+ src.at<unsigned char>(center.y + a - 5, center.x + a) // x+3, y -2
				+ src.at<unsigned char>(center.y + a, center.x - a) // x-3, y +3
				+ src.at<unsigned char>(center.y + a - 1, center.x - a) // x-3, y +2
				+ src.at<unsigned char>(center.y + a - 2, center.x - a) // x-3, y +1
				+ src.at<unsigned char>(center.y + a - 3, center.x - a) // x-3, y 
				+ src.at<unsigned char>(center.y + a - 4, center.x - a) // x-3, y -1
				+ src.at<unsigned char>(center.y + a - 5, center.x - a) // x-3, y -2
				+ src.at<unsigned char>(center.y + a, center.x + a) // x+3, y +3
				+ src.at<unsigned char>(center.y + a, center.x + a - 1) // x+2, y +3
				+ src.at<unsigned char>(center.y + a, center.x + a - 2) // x+1, y +3
				+ src.at<unsigned char>(center.y + a, center.x + a - 3) // x,	y +3
				+ src.at<unsigned char>(center.y + a, center.x + a - 4) // x-1, y +3
				+ src.at<unsigned char>(center.y + a, center.x + a - 5) // x-2, y +3
				+ src.at<unsigned char>(center.y + a, center.x + a) // x+3, y -3
				+ src.at<unsigned char>(center.y - a, center.x + a - 1) // x+2, y -3
				+ src.at<unsigned char>(center.y - a, center.x + a - 2) // x+1, y -3
				+ src.at<unsigned char>(center.y - a, center.x + a - 3) // x,	y -3
				+ src.at<unsigned char>(center.y - a, center.x + a - 4) // x-1, y -3
				+ src.at<unsigned char>(center.y - a, center.x + a - 5); // x-2, y -3

			long mean = count / 24;

			if (mean < maxMeanAroundGlint)
			{
				if (local_debug) { debug_logger() << "mean: " << mean; }
				//rectangle(src, boundRect[i], Scalar(0, 0, 255), 2, 8, 0);
				//circle(src, Point(cx, cy), 1, Scalar(0, 255, 0), 1, 8, 0);
				glintCandidates.emplace_back(contourCenters[i]);
			}
			else
			{
				contourCenters.erase(contourCenters.begin() + i);
			}

			if (local_debug)
			{
				cv::putText(src, std::to_string(i), cv::Point2f(center.x, center.y + 10), cv::FONT_HERSHEY_SIMPLEX, 0.3,
					cv::Scalar(255, 255, 0), 1);
				debug_logger() << "Mean value of " << i << " is " << mean;
			}
		}
	}

	if (local_debug)
	{
		for (int i = 0; i < glintCandidates.size(); i++)
		{
			debug_logger() << "possible glint: " << glintCandidates[i].x << glintCandidates[i].y;
		}
	}
	return glintCandidates;
}

std::vector<cv::Point2f> searchForGlints(cv::Mat src, double firstEyeThresh)
{
	std::vector<cv::Point2f> glints;

	if (src.channels() == 3)
	{
		cv::cvtColor(src, src, CV_BGR2GRAY);
	}

	int kernel_size = 3; // better results in extreme cases
	double scale = 1;
	double delta = 0;
	int ddepth = CV_16S;

	// 1 Gauss
	/// Remove noise by blurring with a Gaussian filter
	/// Blurring also generates wider range for finding contours

	// CHANGED
	// 1
	cv::Mat gaussed;
	cv::GaussianBlur(src, gaussed, cv::Size(3, 3), 0, 0, cv::BORDER_DEFAULT); // better results in extreme cases

																				/// Convert the image to grayscale
	cv::Mat gaussed_gray;
	if (gaussed.channels() == 3)
	{
		cv::cvtColor(gaussed, gaussed_gray, CV_BGR2GRAY);
	}
	else
	{
		gaussed.copyTo(gaussed_gray);
	}

	// 2 Laplace
	/// Apply Laplace function
	/// Laplace Functino generates edges
	cv::Mat laplaced;
	cv::Laplacian(gaussed_gray, laplaced, ddepth, kernel_size, scale, delta, cv::BORDER_DEFAULT);
	
	// CHANGED
	cv::Mat abs_dst;
	cv::convertScaleAbs(laplaced, abs_dst);
	//convertScaleAbs(laplaced, abs_dst, (sigma + 1)*0.25);
	

	
	

	// 3 Find Contours
	/// Detect edges using Threshold
	cv::Mat threshold_output, threshold_output2, threshold_output3;
	cv::threshold(abs_dst, threshold_output, firstEyeThresh, 255, cv::THRESH_BINARY);

	/* NEW APPROACH
	+ Gaussian Smoothing 9x9
	+ square Intensities
	+ threshold really low
	+ morphological closing with 7x7
	*/
	
	// =================
	
	
	//cv::GaussianBlur(threshold_output, threshold_output2, cv::Size(5,5), 0, 0, cv::BORDER_DEFAULT); // better results in extreme cases
	//cv::imshow("Blur", threshold_output2);
	//cv::waitKey(1);

	/*
		cv::Mat squared;
		threshold_output.copyTo(squared);
		squared.convertTo(squared, CV_8UC1);

		for (int y = 0; y < threshold_output.rows; y++)
		{
			for (int x = 0; x < threshold_output.cols; x++)
			{
				squared.at<uchar>(y, x) = (threshold_output.at<uchar>(y, x)) *(threshold_output.at<uchar>(y, x));
			}
		}

		//cv::imshow("sQUARED", squared);
		//cv::waitKey(1);
	

		cv::threshold(squared, threshold_output, 3, 255, cv::THRESH_BINARY);
		*/

	//cv::Mat kernel = cv::Mat::ones(3, 3, CV_8UC1);
	//morphologyEx(threshold_output2, threshold_output3, cv::MORPH_ERODE, kernel);

	//cv::imshow("Out", threshold_output3);
	//cv::waitKey(1);

	
	// =================


	cv::Mat contoursMat;


	src.copyTo(contoursMat);
	cvtColor(contoursMat, contoursMat, CV_GRAY2BGR);


	// FOR SPEED UP
	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector<cv::Point>> contours;
	//cv::findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	cv::findContours(threshold_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	//cv::findContours(threshold_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));


	// 4 Approximate contours to polygons + get bounding rects and circles
	//vector<vector<Point2f> > contours_poly(contours.size());
	std::vector<cv::RotatedRect> minRect(contours.size());
	std::vector<cv::Point2f> contourCenter(contours.size());

	for (int i = 0; i < contours.size(); i++)
	{
		minRect[i] = minAreaRect(cv::Mat(contours[i]));
		contourCenter.emplace_back(minRect[i].center.x, minRect[i].center.y);

		// DEBUG
		//drawContours(contoursMat, contours, i, cv::Scalar(255, 255, 0), 1, 8, hierarchy, 0, cv::Point());
		//circle(contoursMat, cv::Point2f(minRect[i].center.x , minRect[i].center.y), 2, cv::Scalar(0, 0, 255), 1, 8);			//CURRENT
		//qDebug() << minRect[i].center.x << " " << minRect[i].center.y;
	}

	// DEBUG
	//imshow("contoursMat", contoursMat);
	//cv::waitKey(1);
	



	// Detect glints on sclera and remove them from list ======================================================================
	std::vector<cv::Point2f> glintCandidates = removeFalseGlints(contourCenter, threshold_output, src);
	std::vector<cv::Point2f> glintCombi = findGeometry(glintCandidates, src);


	// UNDER CONSTRUCTION: sometimes there are 3 combinations and not the right 2 are used as glints.
	// This function tries to find out which 2 combinations are the right ones
	// by using only the two with the darkest background
	if (glintCombi.size() > 1)
	{
		// 1
		//glintCombi = findMeanAroundGlintCombi(src, glintCombi);
	}


	// no eye found
	if (glintCombi.empty())
	{
		glints = { cv::Point2f(0, 0), cv::Point2f(0, 0), cv::Point2f(0, 0) };
	}
	else
	{
		glints = glintCombi;
		// sort with respect to position in between
		std::sort(glints.begin(), glints.end(), SortCvPoint2fByX());
	}
	
	return glints;

	
}

cv::Point2f calculateCenterOfMass(const std::vector<cv::Point2f>& glints)
{
	if (glints.empty())
		return cv::Point2f(-1, -1);

	int totalX = 0;
	int totalY = 0;

	for (const auto& glint : glints)
	{
		totalX = totalX + glint.x;
		totalY = totalY + glint.y;
	}

	return cv::Point2f(totalX / glints.size(), totalY / glints.size());
}

cv::Rect eyeSubimageFromCoM(cv::Point2f center_of_mass)
{
	if (center_of_mass.x < 0 || center_of_mass.y < 0)
		return cv::Rect(-50, -50, 100, 100);
	return cv::Rect(center_of_mass.x - eye_region_size_x / 2, center_of_mass.y - eye_region_size_y / 2, eye_region_size_x,
		eye_region_size_y);
}


bool cutEyeRegion(const cv::Mat& frame, cv::Mat& output, EyePosition desired_eye, double first_eye_threshold)
{
	const unsigned int aperture_width = frame.cols / 2;

	// Implicit assumption made here: The right eye is in the right half of the image, the left eye in the left half.
	cv::Rect candidate_region = desired_eye == EyePosition::RIGHT ? cv::Rect(0, 0, frame.cols / 2, frame.rows)
		: cv::Rect(frame.cols / 2, 0, frame.cols / 2, frame.rows);
	cv::Mat candidate_image = frame(candidate_region);

	auto glints = searchForGlints(candidate_image, first_eye_threshold);
	cv::Rect eye_region = eyeSubimageFromCoM(calculateCenterOfMass(glints));


	cv::Rect full_boundaries = { 0, 0, candidate_image.cols, candidate_image.rows };
	if (eye_region.tl().inside(full_boundaries) && eye_region.br().inside(full_boundaries))
	{
		output = candidate_image(eye_region);
		
		return true;
	}

	return false;
}



PupilGlintCombiInstance calcPupilGlintCombi(const cv::Point2f& pupilCenterRight, const cv::Point2f& pupilCenterLeft,
	std::vector<cv::Point2f> glintsLeft, std::vector<cv::Point2f> glintsRight)
{



	//3.  COMPUTING Glints and Combination => EyeVec
	// calculate according to old pupil position which glints are visible and save all in 
	// structure curPositions (pupilGlintCombiInstance)
	PupilGlintCombiInstance curPositions;

	// 3. Sort glints for x
	std::sort(glintsLeft.begin(), glintsLeft.end(), SortCvPoint2fByX());
	std::sort(glintsRight.begin(), glintsRight.end(), SortCvPoint2fByX());

	// 4. Calculate glint positions
	switch (glintsLeft.size())
	{
		// one glint visible. 
		// not usable
	case 1:
		if (local_debug) { debug_logger() << "Left: 1 glint detected."; }
		break;

		// only two glints are detected
	case 2:
		if (local_debug) { debug_logger() << "Left: 2 glints detected."; }

		// 1 = leftmost glint
		// 2 = upper middle glint
		if (glintsLeft[0].y < glintsLeft[1].y)
		{
			curPositions.glintLeft_1 = glintsLeft[0];
			curPositions.glintLeft_2 = glintsLeft[1];

			// estimate 3rd glint same y as down glint and  +8 pixel on x axis
			cv::Point2f thirdGlint = cv::Point2f(glintsLeft[1].x + 8, glintsLeft[0].y);

			curPositions.glintLeft_3 = thirdGlint;
		}
		else // 2 = leftmost glint
		{
			// 3 = upper middle glint
			curPositions.glintLeft_2 = glintsLeft[0];
			curPositions.glintLeft_3 = glintsLeft[1];

			// estimate 3rd glint same y as down glint and  -8 pixel on x axis
			cv::Point2f thirdGlint = cv::Point2f(glintsLeft[0].x - 8, glintsLeft[1].y);
			curPositions.glintLeft_1 = thirdGlint;
		}
		break;

		// all three glints are detected
	case 3:
		if (local_debug) { debug_logger() << "Left: 3 Glints detected."; }
		curPositions.glintLeft_1 = glintsLeft[0];
		curPositions.glintLeft_2 = glintsLeft[1];
		curPositions.glintLeft_3 = glintsLeft[2];
		break;

	default:
		if (local_debug)
		{
			debug_logger() << "Left: nothing " << " found." << glintsLeft.size();
		}
		break;
	}

	// right eye
	switch (glintsRight.size())
	{
		// one glint visible. 
		// not usable
	case 1:
		if (local_debug)
		{
			debug_logger() << "Right: 1 glint detected.";
		}
		break;

		// only two glints are detected
	case 2:
		if (local_debug)
		{
			debug_logger() << "Right: 2 glints detected.";
		}

		// 1 = leftmost glint
		// 2 = upper middle glint
		if (glintsRight[0].y < glintsRight[1].y)
		{
			curPositions.glintRight_1 = glintsRight[0];
			curPositions.glintRight_2 = glintsRight[1];

			// estimate 3rd glint same y as down glint and  + 8 pixel on x axis
			cv::Point2f thirdGlint = cv::Point2f(glintsRight[1].x + 8, glintsRight[0].y);
			curPositions.glintRight_3 = thirdGlint;
			glintsRight.push_back(thirdGlint);

			glintsRight[0] = curPositions.glintRight_1;
			glintsRight[1] = curPositions.glintRight_2;
			glintsRight[2] = curPositions.glintRight_3;
		}
		else // 2 = leftmost glint
		{
			// 3 = upper middle glint


			curPositions.glintRight_2 = glintsRight[0];
			curPositions.glintRight_3 = glintsRight[1];

			// estimate 3rd glint same y as down glint and  - 8 pixel on x axis
			cv::Point2f thirdGlint = cv::Point2f(glintsRight[0].x - 8, glintsRight[1].y);
			curPositions.glintRight_1 = thirdGlint;
			glintsRight.push_back(thirdGlint);

			glintsRight[0] = curPositions.glintRight_1;
			glintsRight[1] = curPositions.glintRight_2;
			glintsRight[2] = curPositions.glintRight_3;
		}
		break;

		// all three glints are detected
	case 3:
		if (local_debug)
		{
			debug_logger() << "Right: 3 Glints detected..";
		}
		curPositions.glintRight_1 = glintsRight[0];
		curPositions.glintRight_2 = glintsRight[1];
		curPositions.glintRight_3 = glintsRight[2];
		break;

	default:
		if (local_debug)
		{
			debug_logger() << "Right: nothing " << " found." << glintsRight.size();
		}
		break;
	}

	curPositions.pupilLeft = pupilCenterLeft;
	curPositions.pupilRight = pupilCenterRight;


	

	// MITTWOCH: Check if detection is good

	// if pupil or one of the glints is empty,
	// set counter failedDetections +1
	// else set right detection +1
	if (curPositions.pupilLeft.x <= 0 || curPositions.pupilLeft.y <= 0 || curPositions.glintLeft_1.x <= 0 || curPositions.glintLeft_2.y
		<= 0 || curPositions.glintLeft_2.y <= 0 || curPositions.glintLeft_3.y <= 0 || curPositions.glintLeft_3.y <= 0)
	{
		curPositions.detectedLeft = false;
	}
	else
	{
		curPositions.detectedLeft = true;
	}


	if (curPositions.pupilRight.x <= 0 || curPositions.pupilRight.y <= 0 || curPositions.glintRight_1.x <= 0 ||
		curPositions.glintRight_2.y <= 0 || curPositions.glintRight_2.y <= 0 || curPositions.glintRight_3.y <= 0 ||
		curPositions.glintRight_3.y <= 0)
	{
		curPositions.detectedRight = false;
		curPositions.valid = false;
	}
	else
	{
		curPositions.detectedRight = true;
		curPositions.valid = true;
	}


	if (local_debug)
	{
		debug_logger() << "Left Pupil: " << curPositions.pupilLeft.x << "/" << curPositions.pupilLeft.y;
		debug_logger() << "Right Pupil:" << curPositions.pupilRight.x << "/" << curPositions.pupilRight.y;
	}


	return curPositions;
}

// shallow wrapper for the detection
struct DetectionWrapper
{
	explicit DetectionWrapper(PupilDetection* p, EyePosition e):
		detection(p), eye(e){}
	PupilDetection* detection;
	EyePosition eye;
};


EyeFeatures detectGlintAndPupilSingle(const cv::Mat& frame, const cv::Rect& region, DetectionWrapper pupil_detection,
	double firstEyeThresh, bool precut)
{
	cv::Mat subframe(frame(region));

	ProfilingSection glintPS;
	std::vector<cv::Point2f> glints = searchForGlints(subframe, firstEyeThresh);
	gProfilingMonitor.addTiming("GlintDetection", glintPS);

	//qDebug() << "New image";
	for (int i = 0; i < glints.size(); i++)
	{

		//qDebug() << "Glint " << i << " " << glints[i].x << " " << glints[i].y;
		if (glints[i].x > 0)
		{
			glints[i].x = glints[i].x + region.x;
			glints[i].y = glints[i].y + region.y;
		}

		
	}

	//qDebug() << "\n";

	cv::Rect eye_region = precut? cv::Rect(0,0, subframe.cols, subframe.rows) : eyeSubimageFromCoM(calculateCenterOfMass(glints));
	

	
	
	const cv::Rect full_boundaries = { 0, 0, frame.size().width, frame.size().height };

	if ( (!precut && !(eye_region.tl().inside(full_boundaries) && eye_region.br().inside(full_boundaries))) || frame.empty())
	{
		EyeFeatures features;
		features.glints = glints;
		features.pupil_center = cv::Point2f(0, 0);
		features.detected = false;
		features.is_valid = false;
		return features;
	}

	cv::Mat eye_image = frame(eye_region);

	//cv::imshow("EyeRegion", eye_image);
	//cv::waitKey(1);
	
	ProfilingSection ps;
	auto pupil = pupil_detection.detection->run(eye_image, pupil_detection.eye);
	gProfilingMonitor.addTiming("PupilDetection", ps);

	auto true_pupil_center = cv::Point2f(eye_region.x + pupil.pupil_center.x, eye_region.y + pupil.pupil_center.y);

	EyeFeatures features;
	features.pupil_center = true_pupil_center;
	features.glints = glints;
	features.detected = true;
	features.is_valid = true;
	return features;
}


PupilGlintCombiInstance detectGlintAndPupilNEW(const cv::Mat& frame, double firstEyeThresh, PupilDetection* pupil_detection)
{

	cv::Mat bright;
	frame.copyTo(bright);

	//cv::imshow("detection frame", bright);
	//cv::waitKey(1);

	const unsigned int half_width = (bright.size().width / 2);

	cv::Rect leftRect = cv::Rect(0, 0, half_width, bright.size().height);
	cv::Rect rightRect = cv::Rect(half_width, 0, half_width, bright.size().height);

	

	auto launch_policy = std::launch::deferred;
#ifdef DETECTOR_INNER_ASYNC
	launch_policy = std::launch::async;
#endif

	ProfilingSection pupil_detection_timing;
	auto future_left = std::async(launch_policy, &detectGlintAndPupilSingle, bright, rightRect, DetectionWrapper(pupil_detection, EyePosition::LEFT),
		firstEyeThresh, false);
	auto future_right = std::async(launch_policy, &detectGlintAndPupilSingle, bright, leftRect, DetectionWrapper(pupil_detection, EyePosition::RIGHT),
		firstEyeThresh, false);

	auto features_left = future_left.get();

	auto features_right = future_right.get();


	
	gProfilingMonitor.addTiming("Pupil/Glint Detection / both eyes:", pupil_detection_timing);
	//gProfilingMonitor.addTiming("detectGlintAndPupilSingle Both Eyes", pupil_detection_timing);

	return calcPupilGlintCombi(features_right.pupil_center, features_left.pupil_center, features_left.glints, features_right.glints);
}


PupilGlintCombiInstance detectGlintAndPupilNEWSparse(const SparseInputImage& frame, double firstEyeThresh, PupilDetection* pupil_detection)
{

	auto launch_policy = std::launch::deferred;
#ifdef DETECTOR_INNER_ASYNC
	launch_policy = std::launch::async;
#endif

	ProfilingSection pupil_detection_timing;

	std::future<EyeFeatures> future_left, future_right;
	if (!frame.left_eye.empty()) {
		future_left = std::async(launch_policy, &detectGlintAndPupilSingle, frame.left_eye, cv::Rect(0, 0, frame.left_eye.cols, frame.left_eye.rows),
			DetectionWrapper(pupil_detection, EyePosition::LEFT), firstEyeThresh, true);
	}

	if (!frame.right_eye.empty()) {
		future_right = std::async(launch_policy, &detectGlintAndPupilSingle, frame.right_eye, cv::Rect(0, 0, frame.right_eye.cols, frame.right_eye.rows),
			DetectionWrapper(pupil_detection, EyePosition::RIGHT), firstEyeThresh, true);
	}

	EyeFeatures features_left, features_right;
	if (!frame.left_eye.empty()) {
		features_left = future_left.get();
	}
	if(!frame.right_eye.empty()){
		features_right = future_right.get();
	}

	// Momentan nur bei Calibration Trainer verwendet
	//gProfilingMonitor.addTiming("Pupil- and GlintDetection / both eyes / sparse image:", pupil_detection_timing);

	//gProfilingMonitor.addTiming("detectGlintAndPupilSingleSparse Both Eyes", pupil_detection_timing);

	return calcPupilGlintCombi(features_right.pupil_center, features_left.pupil_center, features_left.glints,
		features_right.glints);
	
}