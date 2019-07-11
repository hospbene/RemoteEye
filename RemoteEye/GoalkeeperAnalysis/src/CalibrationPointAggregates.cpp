#include "CalibrationPointAggregates.h"
#include "Calibration.h"
#include <memory>
#include "ProfilingMonitor.h"

using namespace gazeestimation;

AveragingAggregatedCalibrationPoint::AveragingAggregatedCalibrationPoint(const CalibrationPointData& source, EyePosition eye)
{
	point_on_screen = gazeestimation::make_vec2(source.position[0], source.position[1]);
	raw_data = source.data;

	if (raw_data.empty())
		return;

	// we don't want a default initialized instance with invalid values, but start from 0 for the summing up
	cv::Point2f pupil(0, 0);
	std::vector<cv::Point2f> glints(SingleEyeFeatures::num_glints, cv::Point2f(0, 0));

	std::vector<cv::Point2f> pupils;

	float num_instances = 0;
	for (const auto& point : raw_data)
	{
		const auto& data = point.eye(eye);
		if (!data.allValid())
			continue;
		pupils.push_back(data.pupil_center());
		pupil += data.pupil_center();
		for (size_t i = 0; i < SingleEyeFeatures::num_glints; i++)
		{
			glints[i] += data.glint(i);
		}
		num_instances += 1;
	}

	pupil = cv::Point2f(pupil.x / num_instances, pupil.y / num_instances);

	for (size_t i = 0; i < SingleEyeFeatures::num_glints; i++)
	{
		glints[i] = glints[i] / num_instances;
	}
	aggregate_input = SingleEyeFeatures(pupil, glints);
}

// Geometric Median through Weiszfeld's algorithm
Vec2 geometric_median(const std::vector<Vec2>& points)
{
	std::unique_ptr<double> weights(new double[points.size()]);
	for (int i = 0; i < points.size(); i++)
		weights.get()[i] = 1;

	const int max_iterations = 150;
	int iteration_index = 0;

	Vec2 last_result(-2, -2);
	Vec2 result(-1, -2);

	const double epsilon = 0.0000001;

	while(squared_length_vec2(Vec2(last_result - result)) > epsilon * epsilon)
	{
		last_result = result;
		result = Vec2(0, 0);

		double weights_sum = 0;
		for(int i = 0; i < points.size(); i++)
		{
			const Vec2 diff = last_result - points[i];
			if(length(diff) < 0.001)
			{
				weights.get()[i] = 0;				
			}
			else
			{
				weights.get()[i] = 1 / length(diff);				
			}
			weights_sum += weights.get()[i];
			result += weights.get()[i] * points[i];	
		}

		result /= weights_sum;
		iteration_index++;

		if (iteration_index > max_iterations){
			//TODO: Check how often this happens
			gProfilingMonitor.addTiming("Median2_ERR_conv", ProfilingSection(), true);
			break;
		}
	}
	return result;
}

// Geometric Median through Weiszfeld's algorithm
Vec3 geometric_median3(const std::list<Vec3>& points)
{
	std::unique_ptr<double> weights(new double[points.size()]);
	for (int i = 0; i < points.size(); i++)
		weights.get()[i] = 1;

	const int max_iterations = 150;
	int iteration_index = 0;

	Vec3 last_result(-2, -2, 0);

	Vec3 result = (*points.begin())+Vec3(1,1,1);

	const double epsilon = 0.0005;

	while (squared_length(Vec3(last_result - result)) > epsilon * epsilon)
	{
		last_result = result;
		result = Vec3(0, 0, 0);

		double weights_sum = 0;

		int i = 0;
		for(const auto& point : points)
		{
			const Vec3 diff = last_result - point;


			if (length(diff) < 0.000001)
			{
				weights.get()[i] = 0;
			}
			else
			{
				weights.get()[i] = 1 / length(diff);
			}
			weights_sum += weights.get()[i];
			result += weights.get()[i] * point;
			
			i++;
		}

		result /= weights_sum;

		iteration_index++;

		if (iteration_index > max_iterations) {
			//TODO: Check how often this happens
			gProfilingMonitor.addTiming("Median3_ERR_conv", ProfilingSection(), true);
			break;
		}
	}

	return result;
}

Vec2 cvPoint2Vec2(const cv::Point2f& point)
{
	return { point.x, point.y };
}

cv::Point2f Vec2ToCvPoint2f(const Vec2& vec2)
{
	return cv::Point2f(vec2[0], vec2[1]);
}



cv::Point2f MedianFilter::filter(const cv::Point2f& new_sample) {
	if (!(new_sample.x < 0 || new_sample.y < 0))
	{
		samples.push_back(new_sample);
	}

	while (samples.size() > queue_size)
		samples.erase(samples.begin(), samples.begin() + 1);

	if (samples.empty())
		return cv::Point2f(0, 0);

	if (samples.size() < 2)
		return samples[samples.size()-1];

	std::vector<Vec2> converted;
	for (const auto& point : samples) {
		converted.push_back(cvPoint2Vec2(point));
	}

	const Vec2 median = geometric_median(converted);

	return Vec2ToCvPoint2f(median);
}


MedianAggregatedCalibrationPoint::MedianAggregatedCalibrationPoint(const CalibrationPointData& source, EyePosition eye)
{
	point_on_screen = gazeestimation::make_vec2(source.position[0], source.position[1]);
	raw_data = source.data;

	if (raw_data.empty())
		return;

	std::vector<Vec2> pupils;
	std::vector<std::vector<Vec2>> glints;

	for(size_t i = 0; i < SingleEyeFeatures::num_glints; i++)
	{
		glints.emplace_back();
	}

	float num_instances = 0;
	for (const auto& point : raw_data)
	{
		const auto& data = point.eye(eye);
		if (!data.allValid())
			continue;
		pupils.push_back(cvPoint2Vec2(data.pupil_center()));

		for (size_t i = 0; i < SingleEyeFeatures::num_glints; i++)
		{
			glints[i].push_back(cvPoint2Vec2(data.glint(i)));
		}
		num_instances += 1;
	}

	std::vector<cv::Point2f> glints_result(SingleEyeFeatures::num_glints);
	for(size_t i = 0; i < glints.size(); i++)
	{
		auto median = geometric_median(glints[i]);
		glints_result[i] = Vec2ToCvPoint2f(median);
	}
	
	aggregate_input = SingleEyeFeatures(Vec2ToCvPoint2f(geometric_median(pupils)), glints_result);
	
}

AggregatedCalibrationData::AggregatedCalibrationData(const CalibrationData& data, EyePosition eye, AggregationMode mode) :
	eye(eye)
{
	for (const auto& point : data.calibration_points)
	{
		if(mode == AggregationMode::AVERAGE)
			points.emplace_back(AveragingAggregatedCalibrationPoint(point, eye));
		else if (mode == AggregationMode::MEDIAN)
			points.emplace_back(MedianAggregatedCalibrationPoint(point, eye));
	}
}

Vec3MedianFilter::Vec3MedianFilter(unsigned int size):
data(),
size(size)
{
	
}

double median(std::vector<double> values)
{
	if(values.size() <= 0)
	{
		return 0;
	}
	if(values.size() == 1 || values.size() == 2)
	{
		return values[1];
	}

	std::sort(values.begin(), values.end());
	return values[values.size() / 2];
}



gazeestimation::Vec3 Vec3MedianFilter::newSample(gazeestimation::Vec3 new_sample)
{
	data.push_back(new_sample);
	
	while(data.size() > size){
		data.pop_front();
	}

	if(data.size() < 3)
	{
		return new_sample;
	}

	return geometric_median3(data);
}

void Vec3MedianFilter::reset()
{
	data.clear();
}
