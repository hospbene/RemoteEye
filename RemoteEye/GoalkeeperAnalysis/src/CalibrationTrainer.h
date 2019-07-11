#ifndef CALIBRATION_TRAINER_H_
#define CALIBRATION_TRAINER_H_


#include "IntermediateTypes.h"
#include <boost/noncopyable.hpp>
#include "SparseInputImage.h"

class BoreWrapper;

/// Some of the pupil detection algorithms yield improved results when a dedicated
/// training procedure during the calibration is adhered to. This interface provides
/// a way to integrate this into the calibration process.
class CalibrationTrainer : public boost::noncopyable
{
public:
	typedef std::function<DetectedFeatures(const cv::Mat&)> FeatureDetection;
	typedef std::function<DetectedFeatures(const SparseInputImage&)> FeatureDetectionSparse;

	virtual ~CalibrationTrainer() = default;
	virtual void receiveData(QueuedFrame frame) = 0;

	/// Fills out the provided calibration data instance with the actual data.
	/// 
	/// \param data The start and end times for each of the calibration points within this must be
	///				set correctly. Any images on record that do not fit into one of these times
	///				will be used for training, but will not result in an insertion of a DetectedFeatures 
	///				instance into this parameter.
	/// \note This is a performance intensive operation and may take several seconds to complete
	virtual void enrich(CalibrationData& data) = 0;

	/// Does this training method overwrite the detected features completely? If this function
	/// returns true, then this signifies that the calibration process is free to not do any 
	/// feature detection.
	virtual bool overwritesFeatureDetection() const = 0;
};

class NoTraining : public CalibrationTrainer
{
public:
	void receiveData(QueuedFrame frame) override;
	void enrich(CalibrationData& data) override;
	bool overwritesFeatureDetection() const override;
};


class BoreTrainer : public CalibrationTrainer
{
public:
	/// The eye separation must return a pair of (left_eye_image, right_eye_image), where
	/// the left and right eye images correspond to the eye image dimensions as specified in constants.h
	/// If no eye could be located for that position on the input image, then an empty mat must be returned
	typedef std::function<std::pair<cv::Mat, cv::Mat>(const cv::Mat&)> EyeSeparation;

	explicit BoreTrainer(BoreWrapper* bore, CalibrationTrainer::FeatureDetectionSparse feature_detection, EyeSeparation eye_separation, int max_train_samples_per_sec);

	void receiveData(QueuedFrame frame) override;
	void enrich(CalibrationData& data) override;
	bool overwritesFeatureDetection() const override;

private:
	BoreWrapper* const bore;
	std::vector<QueuedSparseInput> stored_images;
	CalibrationTrainer::FeatureDetectionSparse feature_detection;
	EyeSeparation eye_separation;
	int max_training_samples_per_sec;
};

#endif