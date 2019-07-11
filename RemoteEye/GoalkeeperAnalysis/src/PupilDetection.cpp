#include "PupilDetection.h"
#include "algo.h"
#include "bore.h"

#include "IntermediateTypes.h"
#include "elifitter.h"
#include "constants.h"
#include "DetectorCode.h"
#include "utils.h"

#include <opencv2/videoio.hpp>

PupilDetectionResult PupilDetection::run(const cv::Mat& frame, EyePosition position)
{
	PupilDetectionResult res;
	res.pupil_center = cv::Point2f(-100, -100);
	return res;
}

PupilDetectionResult ElseWrapper::run(const cv::Mat& frame, EyePosition position)
{
	cv::RotatedRect res = position == EyePosition::RIGHT
		                      ? ELSE::run_fast_idx_1(frame)
		                      : ELSE::run_fast_idx_2(frame);

	PupilDetectionResult pres;
	pres.pupil_center = res.center;
	return pres;
}

void deleteBore(BORE* b)
{
	b->m_clear();
	delete b;
}

BoreWrapper::BoreWrapper():
	bore_left(new BORE(), deleteBore),
	bore_right(new BORE(), deleteBore)
{
	bore_left->set_INPUT_SIZE(eye_region_size_x, eye_region_size_y);
	bore_right->set_INPUT_SIZE(eye_region_size_x, eye_region_size_y);

	bore_left->m_init_without_file(6, 15, 2, 3.0f, 1);
	bore_right->m_init_without_file(6, 15, 2, 3.0f, 1);
	//bore_left->m_load("ELSE_trainingFiles\\new_detector.txt");
	bore_left->cmp_indexes();
	bore_right->cmp_indexes();
}

PupilDetectionResult BoreWrapper::run(const cv::Mat& frame, EyePosition eye)
{
	ellipse_out res = eye==EyePosition::RIGHT?bore_right->run_fast_idx(frame) : bore_left->run_fast_idx(frame);

	PupilDetectionResult pres;
	if (!res.valid)
		pres.pupil_center = cv::Point2f(-1, -1);
	else
		pres.pupil_center = res.eli.center;
	return pres;
}


bool cutEyeRegion_tmp(const cv::Mat& frame, cv::Mat& output, EyePosition desired_eye, double first_eye_threshold)
{
	const unsigned int aperture_width = frame.cols / 2;

	// Implicit assumption made here: The right eye is in the right half of the image, the left eye in the left half.
	cv::Rect candidate_region = desired_eye == EyePosition::LEFT ? cv::Rect(0, 0, frame.cols / 2, frame.rows)
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
void BoreWrapper::train(const std::vector<std::pair<cv::Mat, cv::Mat>>&  mats)
{
#ifdef BORE_TRAINING_SAVE_VIDEO
	ensureDirectoryExists(bore_traning_video_folder);
	cv::VideoWriter bore_training_video(bore_traning_video_folder + "/training_eye_right.avi", lossless_video_codec, 10, cv::Size(eye_region_size_x*3, eye_region_size_y), true);

	std::vector<cv::Mat> training_video_frames;

	int valid_right_eye_images=0, valid_left_eye_images=0;
	for (int i = 0; i < mats.size(); i++)
	{
		// get the right eye image only for now
		cv::Mat eye_image_right = mats[i].second;
		cv::Mat video_frame(cv::Size(eye_region_size_x * 3, eye_region_size_y), CV_8UC3, cv::Scalar(0, 0, 0));

		if (!eye_image_right.empty()){
			valid_right_eye_images++;
			auto pos = bore_right->run_fast(eye_image_right);
			cv::Mat frame_before;
			cv::cvtColor(eye_image_right, frame_before, CV_GRAY2BGR);
			cv::Mat third_frame_roi  = video_frame(cv::Rect(2 * eye_region_size_x, 0, eye_region_size_x, eye_region_size_y));
			frame_before.copyTo(third_frame_roi);

			if (pos.valid){
				cv::circle(frame_before, pos.eli.center, 1, cv::Scalar(0, 0, 255), -1);
				cv::circle(video_frame, pos.eli.center+cv::Point2f(eye_region_size_x*2, 0), 2, cv::Scalar(0, 0, 255), -1);
			}

			cv::Mat video_frame_roi = video_frame(cv::Rect(0, 0, eye_region_size_x, eye_region_size_y));
			frame_before.copyTo(video_frame_roi);
		}
		training_video_frames.push_back(video_frame);
	}
	for (int i = 0; i < mats.size(); i++)
	{
		if (!mats[i].first.empty())
			valid_left_eye_images++;
	}

	qDebug() << "valid_right_eye_images: " << valid_right_eye_images;
	qDebug() << "valid_left_eye_images : " << valid_left_eye_images;
	qDebug() << "total           images: " << mats.size();
#endif

	for(int i = 0; i < mats.size(); i++)
	{
		// get the right eye image only for now
		cv::Mat eye_image_right = mats[i].second;
		cv::Mat eye_image_left = mats[i].first;

		if(!eye_image_right.empty()){
			auto pos = bore_right->run_fast(eye_image_right);
			bore_right->run_fast_train(eye_image_right, pos, 3.0f);
		}

		if (!eye_image_left.empty())
		{
			auto pos = bore_left->run_fast(eye_image_left);
			bore_left->run_fast_train(eye_image_left, pos, 3.0f);
		}
	}


	bore_right->m_norm_weight(0.9);
	bore_right->cmp_indexes();
	bore_left->m_norm_weight(0.9);
	bore_left->cmp_indexes();


#ifdef BORE_TRAINING_SAVE_VIDEO
	
	for (int i = 0; i < mats.size(); i++)
	{
		cv::Mat existing_frame = training_video_frames[i];

		cv::Mat eye_image_right = mats[i].second;
		if (!eye_image_right.empty())
		{
			auto pos = bore_right->run_fast(eye_image_right);
			cv::Mat frame_before;
			cv::cvtColor(eye_image_right, frame_before, CV_GRAY2BGR);

			if (pos.valid) {
				cv::circle(frame_before, pos.eli.center, 1, cv::Scalar(0, 255, 0), -1);
				cv::circle(existing_frame, pos.eli.center + cv::Point2f(eye_region_size_x * 2, 0), 1, cv::Scalar(0, 255, 0), -1);
			}

			cv::Mat video_frame_roi = existing_frame(cv::Rect(eye_region_size_x, 0, eye_region_size_x, eye_region_size_y));
			frame_before.copyTo(video_frame_roi);
		}

		bore_training_video << existing_frame;
	}

	bore_training_video.release();
#endif
}
