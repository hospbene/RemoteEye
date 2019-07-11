#ifndef PUPIL_DETECTION_H_
#define PUPIL_DETECTION_H_

#include <opencv2/core.hpp>
#include <memory>


enum class EyePosition;
class BORE;

struct PupilDetectionResult
{
	cv::Point2f pupil_center;
};

/// The pupil detection method must support two calls in parallel for the left and right eye
class PupilDetection
{
public:
	virtual ~PupilDetection() = default;
	virtual PupilDetectionResult run(const cv::Mat& frame, EyePosition position);
};

class ElseWrapper : public PupilDetection
{
public:
	PupilDetectionResult run(const cv::Mat& frame, EyePosition position) override;
};


class BoreWrapper : public PupilDetection
{
public:
	explicit BoreWrapper();
	PupilDetectionResult run(const cv::Mat& frame, EyePosition position) override;
	void train(const std::vector<std::pair<cv::Mat, cv::Mat>>&  training_data);

private:
	std::unique_ptr<BORE, void(*)(BORE*)> bore_left;
	std::unique_ptr<BORE, void(*)(BORE*)> bore_right;
};


#endif