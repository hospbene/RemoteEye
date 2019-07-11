#ifndef CALIBRATION_POINT_AGGREGATES_H_
#define CALIBRATION_POINT_AGGREGATES_H_

#include "IntermediateTypes.h"
#include "constants.h"
#include <queue>
#include <list>

class AggregatedCalibrationPoint
{
protected:
	AggregatedCalibrationPoint() = default;

public:
	gazeestimation::Vec2 point_on_screen;
	std::vector<DetectedFeatures> raw_data;
	SingleEyeFeatures aggregate_input;
};

class AveragingAggregatedCalibrationPoint : public AggregatedCalibrationPoint
{
public:
	explicit AveragingAggregatedCalibrationPoint(const CalibrationPointData& source, EyePosition eye);
};

class MedianAggregatedCalibrationPoint : public AggregatedCalibrationPoint
{
public:
	explicit MedianAggregatedCalibrationPoint(const CalibrationPointData& source, EyePosition eye);
};

struct AggregatedCalibrationData
{
	explicit AggregatedCalibrationData(const CalibrationData& data, EyePosition eye, AggregationMode mode);

	std::vector<AggregatedCalibrationPoint> points;
	const EyePosition eye;
};

class Vec3MedianFilter
{
public:
	explicit Vec3MedianFilter(unsigned int size);
	gazeestimation::Vec3 newSample(gazeestimation::Vec3 new_sample);
	void reset();

private:
	std::list<gazeestimation::Vec3> data;
	unsigned int size;
};



class MedianFilter {
public:
	cv::Point2f filter(const cv::Point2f& new_sample);
private:
	const unsigned int queue_size = 20;
	std::vector<cv::Point2f> samples;
};

#endif