#include <boost/filesystem.hpp>
#include "OutputMethods.h"

#include <fstream>
#include <MAFilter.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/videoio/videoio_c.h>

#include <opencv2/highgui/highgui.hpp>

#include <kalman.h>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QDebug>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


#include "constants.h"
#include "DetectorCode.h"

bool writeUserVideoFile(const std::string& filename, const VideoFrame2Timestamp& userVideoAssociation)
{
	// Write UservideoFile 
	std::ofstream userVideoFile;
	userVideoFile.open(filename);

	if (!userVideoFile.is_open())
	{
		return false;
	}

	userVideoFile << "Start: " << userVideoAssociation.start_time << "\n";

	for (const auto value : userVideoAssociation.frame2time)
	{
		userVideoFile << "Frame " << value.first << ": " << value.second << "\n";
	}

	userVideoFile << "End: " << userVideoAssociation.end_time;

	userVideoFile.close();

	return true;
}


VideoFrame2Gaze estimateAndSaveEyeVideoFile(const std::string& eye_video,
	const std::string& output_filename,
	const VideoFrame2Timestamp& eyeVideoDictionary,
	DetectionToGazeEstimation estimation,
	PupilDetection* pupil_detection,
	int first_eye_threshold) 
{
	VideoFrame2Gaze result;
	std::ofstream eyeFile(output_filename);

	// ==================================
	MAFilter pupilR_maFilter(MAFilter(20, true, 1));
	MAFilter pupilL_maFilter(MAFilter(20, true, 1));
	// ==================================


	// analyze eye video
	// detect Glints and Pupil for each frame of eye video
	cv::VideoCapture eyeVid = cv::VideoCapture::VideoCapture(eye_video);
	long frameCount = 0;
	cv::Mat frame1, frame2;	
	qDebug() << "Processing saved EyeVideos... this may take some minutes.";

	eyeFile << "Start: " << eyeVideoDictionary.start_time << std::endl;
	qDebug() << "Total amount of frames saved: " << eyeVid.get(CV_CAP_PROP_FRAME_COUNT);
	qDebug() << "Average detection FPS: " << (eyeVideoDictionary.end_time - eyeVideoDictionary.start_time) / eyeVid.get(CV_CAP_PROP_FRAME_COUNT);
	qDebug() << "Eyevideo START-Timestamp: " << eyeVideoDictionary.start_time;
	qDebug() << "Creating eyefeature detection file.";

	// write header for eyeFile
	eyeFile << "timestamp " << "pupil_right " << "pupil_left " << "glint1_right " << "glint2_right " << "glint3_right "
		<< "glint1_left " << "glint2_left " << "glint3_left " << "gaze_x " << "gaze_y " << std::endl;

	// walk though each frame find glints and pupil.
	// estimate gaze and save gaze in eyeVideoDictionary at right position according to frame number
	//while (eyeVid.get(CV_CAP_PROP_POS_FRAMES) < eyeVid.get(CV_CAP_PROP_FRAME_COUNT) - 1)
	while (1)
	{
		qDebug() << "FrameCount: " << frameCount;
		eyeVid.read(frame1);

		if (frame1.empty())
			break;

		eyeVid.set(CV_CAP_PROP_POS_FRAMES, frameCount + 1);

		frameCount += 1;

		PupilGlintCombiInstance pgci = detectGlintAndPupilNEW(frame1, first_eye_threshold, pupil_detection);

		// ================================
		// GLINTS: Kalman Filter
		// PUPIL:  MAFilter

		PupilGlintCombiInstance ergebnis = pgci;

		
		// right Pupil
		if (pgci.pupilRight.x > 0 && pgci.pupilRight.y > 0)
		{
			ergebnis.pupilRight = pupilR_maFilter.movingAveragesFilter(pgci.pupilRight);
		}
		

		if (ergebnis.glintRight_1.x > 0 && ergebnis.glintRight_1.y)
		{
			KALMAN::GlintFilter::correct(KALMAN::GlintFilter::RIGHT_0, ergebnis.glintRight_1.x, ergebnis.glintRight_1.y);
		}


		if (ergebnis.glintRight_2.x > 0 && ergebnis.glintRight_2.y > 0)
		{
			KALMAN::GlintFilter::correct(KALMAN::GlintFilter::RIGHT_1, ergebnis.glintRight_2.x, ergebnis.glintRight_2.y);

		}


		if (ergebnis.glintRight_3.x > 0 && ergebnis.glintRight_3.y > 0)
		{
			KALMAN::GlintFilter::correct(KALMAN::GlintFilter::RIGHT_2, ergebnis.glintRight_3.x, ergebnis.glintRight_3.y);

		}


		
		// left Pupil
		if (pgci.pupilLeft.x > 0 && pgci.pupilLeft.y > 0)
		{
			ergebnis.pupilLeft = pupilL_maFilter.movingAveragesFilter(pgci.pupilLeft);
			//qDebug() << "New Pupilleft: " << ergebnis.pupilLeft.x << " " << ergebnis.pupilLeft.y;
		}
	

		if (ergebnis.glintLeft_1.x > 0 && ergebnis.glintLeft_1.y > 0)
		{
			KALMAN::GlintFilter::correct(KALMAN::GlintFilter::LEFT_0, ergebnis.glintLeft_1.x, ergebnis.glintLeft_1.y);

		}


		if (ergebnis.glintLeft_2.x > 0 && ergebnis.glintLeft_2.y > 0)
		{
			KALMAN::GlintFilter::correct(KALMAN::GlintFilter::LEFT_1, ergebnis.glintLeft_2.x, ergebnis.glintLeft_2.y);

		}


		if (ergebnis.glintLeft_3.x > 0 && ergebnis.glintLeft_3.y > 0)
		{
			KALMAN::GlintFilter::correct(KALMAN::GlintFilter::LEFT_2, ergebnis.glintLeft_3.x, ergebnis.glintLeft_3.y);

		}
		

		auto gaze = estimation(DetectedFeatures(pgci));
		// TO-DO //auto gaze = estimation(DetectedFeatures(ergebnis));

		//qDebug() << "FilteredGaze: " << gaze.position.x << " " << gaze.position.y;
		// ================================
		//auto gaze = estimation(DetectedFeatures(pgci));
		// ================================


		// ================================
		// GAZE: Kalman Filter 
		cv::Point2f gazePostition = gaze.position;
		// TO-DO KALMAN::GlintFilter::correct(KALMAN::GlintFilter::GAZE, gazePostition.x, gazePostition.y, 100.0, 100.0, 40.0);

		ScreenspaceGazeResult filteredGaze(gazePostition);

		qDebug() << "Gaze:" << gaze.position.x << "/" << gaze.position.y;
		if (filteredGaze.valid)
			// TO-DO //result.frame2gaze.emplace(eyeVid.get(CV_CAP_PROP_POS_FRAMES), filteredGaze.position);
			result.frame2gaze.emplace(eyeVid.get(CV_CAP_PROP_POS_FRAMES), gaze.position);
		if (!filteredGaze.valid)
			result.frame2gaze.emplace(eyeVid.get(CV_CAP_PROP_POS_FRAMES), cv::Point2f(1,1));
		// TO-DO //result.frame2gaze.emplace(eyeVid.get(CV_CAP_PROP_POS_FRAMES), cv::Point2f(-1e6, -1e6));
		// ================================

		/*if(gaze.valid)
			result.frame2gaze.emplace(eyeVid.get(CV_CAP_PROP_POS_FRAMES), gaze.position);
		if (gaze.valid)
			result.frame2gaze.emplace(eyeVid.get(CV_CAP_PROP_POS_FRAMES), cv::Point2f(-1e6,-1e6));
			*/

			// ================================

		// save infos to file
		double time = eyeVideoDictionary.frame2time.at(frameCount);

		eyeFile << time << " " << pgci.pupilRight << " " << pgci.pupilLeft << "  " << pgci.glintRight_1 << " "
		<< pgci.glintRight_2 << " " << pgci.glintRight_3 << " " << pgci.glintLeft_1 <<
		" " << pgci.glintLeft_2 << " " << pgci.glintLeft_3 << gaze.position.x << " " << gaze.position.y << std::endl;
	}


	eyeFile << "End: " << eyeVideoDictionary.end_time << std::endl;

	qDebug() << "Eyevideo END-Timestamp: " << eyeVideoDictionary.end_time;

	// close file
	if (eyeFile.is_open())
	{
		eyeFile.close();
	}


	qDebug() << "Finished processing EyeVideos.";

	return result;
}


bool drawGaze(const std::string& source_video, const std::string& output_file,
	const VideoFrame2Timestamp& stimulus_meta,
	const VideoFrame2Timestamp& eye_meta,
	const VideoFrame2Gaze& gaze_meta,
	const cv::Rect& stimulus_playback_rect)
{
	qDebug() << "Creating output video file...";
	cv::VideoCapture stimulus = cv::VideoCapture::VideoCapture(source_video);
	
	if (!stimulus.isOpened())
	{
		qWarning() << "drawGaze: Couldn't open reader from " << source_video.c_str();
		//qWarning() << "drawGaze: Couldn't open reader from " << stimulus.c_str();
		return false;
	}


	int stimulus_width = stimulus.get(CV_CAP_PROP_FRAME_WIDTH);
	int stimulus_height = stimulus.get(CV_CAP_PROP_FRAME_HEIGHT);
	cv::Size stimulus_size = cv::Size(stimulus_width, stimulus_height);

	int userVideoFPS = stimulus.get(CV_CAP_PROP_FPS);

	// Original
	//cv::VideoWriter output_writer(output_file, lossy_video_codec, userVideoFPS, stimulus_size);
	
	//Works
	//cv::VideoWriter output_writer("output.avi", -1, 30, cv::Size(1904,1004));

	cv::VideoWriter output_writer(output_file, -1, userVideoFPS, stimulus_size, true);

	if (!output_writer.isOpened())
	{

		qWarning() << "drawGaze: Couldn't open outwards writer to " << output_file.c_str();
		qWarning() << "output_file: " << output_file.c_str() << ", lossy_video_codec: " << lossy_video_codec << ", FPS: " << userVideoFPS << ", stimulus Size: width: " << stimulus_width << " height: " << stimulus_height;
		stimulus.release();
		return false;
	}
	
	qDebug() << "Synchronizing estimated gaze points timestamps with stimulus... ";
	qDebug() << "Drawing gaze points...";
	drawGaze(&stimulus, &output_writer, stimulus_meta, eye_meta, gaze_meta, stimulus_playback_rect);
	qDebug() << "Finished drawing gaze points";

	stimulus.release();
	output_writer.release();   // not needed as the destructor of the videowriter already calls that function
	return true;
}

void drawGaze(cv::VideoCapture* stimulus, cv::VideoWriter* output_writer,
	const VideoFrame2Timestamp& stimulus_meta,
	const VideoFrame2Timestamp& eye_meta,
	const VideoFrame2Gaze& gaze_meta,
	const cv::Rect& stimulus_playback_rect)
{
	int userVideoWidth = stimulus->get(CV_CAP_PROP_FRAME_WIDTH);
	int userVideoHeight = stimulus->get(CV_CAP_PROP_FRAME_HEIGHT);
	const double scalex = userVideoWidth / static_cast<double>(stimulus_playback_rect.width);
	const double scaley = userVideoHeight / static_cast<double>(stimulus_playback_rect.height);

	cv::Size output_size = cv::Size(userVideoWidth, userVideoHeight);

	stimulus->set(CV_CAP_PROP_POS_FRAMES, 0);

	while (stimulus->get(CV_CAP_PROP_POS_FRAMES) < stimulus->get(CV_CAP_PROP_FRAME_COUNT) - 1)
	{
		// 1. read frame from user video
		cv::Mat img;
		stimulus->read(img);

		if (stimulus_meta.frame2time.find(stimulus->get(CV_CAP_PROP_POS_FRAMES)) == stimulus_meta.frame2time.end()
			|| stimulus_meta.frame2time.find(stimulus->get(CV_CAP_PROP_POS_FRAMES) + 1) == stimulus_meta.frame2time.end())
		{
			cv::putText(img, "Stimulus Frame2Time Data missing.", cv::Point2f(20, 100), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255, 0, 0), 3);
			(*output_writer) << img;
			continue;
		}

		long currentUserFrameTime = stimulus_meta.frame2time.at(stimulus->get(CV_CAP_PROP_POS_FRAMES));
		long nextUserFrameTime = stimulus_meta.frame2time.at(stimulus->get(CV_CAP_PROP_POS_FRAMES) + 1);

		// 2. get corresponding gaze find all frames which timestamp is greater than the one from the user video but smaller than the NEXT timestamp of the uservideo frame timestamp

		//for (int i = 0; i < eye_meta.frame2time.size(); i++)
		// test
		for (int i = 0; i < eye_meta.frame2time.size(); i++)
		{
			if (eye_meta.frame2time.at(i) >= currentUserFrameTime && eye_meta.frame2time.at(i) < nextUserFrameTime)
			{
				// 3. draw gaze on it
				// check if index i is valid by checking count(). Count returns 1 if mapping from key i exists
				if(gaze_meta.frame2gaze.count(i))
				{

					const cv::Point2f pog_screenspace = gaze_meta.frame2gaze.at(i);
					const cv::Point2f pog_translated = cv::Point2f(pog_screenspace.x - stimulus_playback_rect.tl().x,
						pog_screenspace.y - stimulus_playback_rect.tl().y);
					const cv::Point2f point_of_gaze(pog_translated.x * scalex, pog_translated.y * scaley);

					if (point_of_gaze.x < 0 || point_of_gaze.y < 0 || std::isnan(point_of_gaze.x) || std::isnan(point_of_gaze.y))
					{
						cv::putText(img, "No detection.", cv::Point2f(20, 100), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255, 0, 0), 3);
						//cv::drawMarker(img, Point(10,10), cv::Scalar(255, 0, 255), MARKER_CROSS, 40, 2);
					}
					else if (point_of_gaze.x > output_size.width || point_of_gaze.y > output_size.height)
					{
						cv::putText(img, "Outside bounds.", cv::Point2f(20, 100), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255, 0, 0), 3);
					}
					else
					{
						//cv::drawMarker(img, gaze2frame[i], cv::Scalar(255, 255, 0), MARKER_CROSS, 40, 2);
						qDebug() << "drawing gaze at " << gaze_meta.frame2gaze.at(i).x << " / " << gaze_meta.frame2gaze.at(i).y;
						cv::circle(img, gaze_meta.frame2gaze.at(i), 10, cv::Scalar(0, 0, 255), CV_FILLED, 8, 0);
					}
					
					// 4. save frame in new file
					(*output_writer) << img;

					qDebug() << "drawing gaze" << i;
					cv::imshow("Frame1", img);
					cv::waitKey(1);

				}
				else
				{
					break;
				}
			}
		}
	}

}


void drawFeatures(cv::Mat& frame, const DetectedFeatures& features, EyePosition eye)
{
	SingleEyeFeatures our_features = features.eye(eye);
	

	// glints
	for (size_t i = 0; i < SingleEyeFeatures::num_glints; i++)
	{
		if (!our_features.glintValid(i))
			continue;

		// first glint gets special drawing
		cv::Scalar color = i == 0 ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
		cv::circle(frame, our_features.glint(i), 2, color, 1, 8, 0);
		cv::drawMarker(frame, our_features.glint(i), color, cv::MARKER_CROSS, 5, 1);
	}

	if(our_features.pupilValid())
	{
		cv::drawMarker(frame, our_features.pupil_center(), cv::Scalar(0, 255, 0), cv::MARKER_CROSS, 10, 1);
	}

}

void drawFeatures(cv::Mat& frame, const DetectedFeatures& instance)
{
	drawFeatures(frame, instance, EyePosition::RIGHT);
	drawFeatures(frame, instance, EyePosition::LEFT);
}
