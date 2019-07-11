#include "CalibrationTrainer.h"
#include <utility>
#include "PupilDetection.h"
#include "DetectorCode.h"

#include <opencv2/imgproc.hpp>
#include <cameraCalibration.h>
#include "constants.h"
#include "ProfilingMonitor.h"

void NoTraining::receiveData(QueuedFrame frame)
{
	// intentionally left empty
	return;
}

void NoTraining::enrich(CalibrationData& data)
{
	// intentionally left empty
	return;
}

bool NoTraining::overwritesFeatureDetection() const
{
	return false;
}

BoreTrainer::BoreTrainer(BoreWrapper* bore,
                         CalibrationTrainer::FeatureDetectionSparse feature_detection, EyeSeparation eye_separation,
                         int max_train_samples_per_sec):
	bore(bore),
	feature_detection(std::move(feature_detection)),
	eye_separation(eye_separation),
	max_training_samples_per_sec(max_train_samples_per_sec) { }

void BoreTrainer::receiveData(QueuedFrame frame)
{
	auto split = eye_separation(frame.frame);
	QueuedSparseInput s;

	// These must be copied to a new image, if just the region is taken then the 
	// reference counter on the original image is not decreased, defeating the purpose
	// of only saving a sparse version of that image in the first place.
	//TODO: Variable format.
	s.image.left_eye = cv::Mat(split.first.rows, split.first.cols, CV_8UC1);
	split.first.copyTo(s.image.left_eye);
	s.image.right_eye = cv::Mat(split.second.rows, split.second.cols, CV_8UC1);
	split.second.copyTo(s.image.right_eye);
	s.origin_time = frame.origin_time;

	stored_images.push_back(s);
}

void BoreTrainer::enrich(CalibrationData& data)
{
	// train
	std::vector<std::pair<cv::Mat, cv::Mat>> training_images;


	// if a limitation of training data is desired, we don't want to only train on the first x images, as that
	// will not represent a good sample of pupil positions & glints. Instead go through time images and make sure that 
	// for each second there's only so many images. This also helps by making sure that when there was a lot of trouble
	// detecting the pupil during one second, most of the available samples are used for training, whereas in cases
	// where there was no problem even without, only so many are used.
	std::map<FrameTimestamp, int> counter;
	for (const auto& stored_image : stored_images)
	{
		FrameTimestamp rounded = (stored_image.origin_time / 1000) * 1000;

		int count = 0;
		auto lookup = counter.find(rounded);
		if (lookup != counter.end())
		{
			count = lookup->second;
		}

		count++;
		counter.insert_or_assign(rounded, count);

		if (count - 1 < max_training_samples_per_sec)
		{
			training_images.emplace_back(stored_image.image.left_eye, stored_image.image.right_eye);
		}
	}
	
	bore->train(training_images);

	training_images.erase(training_images.begin(), training_images.end());


	// delete any old data
	for (auto& point : data.calibration_points)
	{
		point.data.erase(point.data.begin(), point.data.end());
	}

	// reevaluate the data
	for (int i = 0; i < stored_images.size(); i++)
	{
		auto frame = stored_images[i];

		auto features = feature_detection(frame.image);

		// assign to the correct point by time
		for (auto& point : data.calibration_points)
		{
			if (frame.origin_time >= point.start_time && frame.origin_time < point.end_time)
			{
				point.data.push_back(features);
				break;
			}
		}
	}

	stored_images.erase(stored_images.begin(), stored_images.end());
}

bool BoreTrainer::overwritesFeatureDetection() const
{
	return true;
}
