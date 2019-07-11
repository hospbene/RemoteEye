
#ifndef CALIBRATION_H_
#define CALIBRATION_H_


#include "boost/tuple/tuple.hpp"

#include "IntermediateTypes.h"
#include "../../../GazeEstimationCpp/MathTypes.hpp"
#include "../../../GazeEstimationCpp/GazeEstimationTypes.hpp"

#include "CalibrationPointAggregates.h"

/// \file Note that all of themse methods assume, if not otherwise noted, that the calibration points are given in order of time
///		and that the data within each point is ordered in the order it was recorded on.

struct CalibrationAccuracyRecord
{
	explicit CalibrationAccuracyRecord(const cv::Point2f& center) :
		centerOfCrossOnScreen(center) {}

	const cv::Point2f centerOfCrossOnScreen;
	cv::Point2f averageEstimate;

	std::vector<cv::Point2f> allEstimates;

	double averagePrecision = 0.0;
	double averageAccuracy = 0.0;
	double meanAccuracy = 0.0;
	double stdPrec = 0.0;
};


struct CompleteCalibrationEvaluation
{
	explicit CompleteCalibrationEvaluation(AggregatedCalibrationData aggregate_right,
	                                       AggregatedCalibrationData aggregate_left,
	                                       const CalibrationData& raw_data);

	AggregatedCalibrationData aggregate_right;
	AggregatedCalibrationData aggregate_left;

	std::vector<CalibrationAccuracyRecord> point_accuracies;

	double averageAccuracy = 0.0;
	double averagePrecision = 0.0;

		

	double totalDetectionRateLEFT = 0;
	double totalUnDetectionRateLEFT = 0;

	double totalDetectionRateRIGHT = 0;
	double totalUnDetectionRateRIGHT = 0;
		
	double totalDetectionRate = 0;
	double totalUnDetectionRate =0;

	double totalFrameCount = 0;

	double percentageDetection = 0.0;
	double percentageDetectionRight = 0.0;
	double percentageDetectionLeft = 0.0;


	double meanSTDPrecision = 0.0;
	double meanMEANAccuracy = 0.0;

	CalibrationData raw_data;

	bool writeToFile(const std::string& filename);
	bool writeImage(const std::string& filename);
	bool writeInputEvluationImage(const std::string& filename, const std::string& glints_filename);

	void writeEstimationsToFile(const std::string & filename);

	void writeRawData(const std::string & filename);

	void writeFeatureDetectionRate(const std::string & filename);

};

double visualAngle(cv::Point2f point1, cv::Point2f point2, float screen_pixel_size_x, float screen_pixel_size_y,
                   float true_screen_distance);

typedef std::function<void()> PointJumpCallback;
/// Evaluates the accuracy of the gaze estimation on the given (possible pre-filtered) calibration input data
///	NOTE: If the given estimation_function does apply any kind of filter or is otherwise not side-effect free,
/// then both the jump_callback must be used, and the given input data for each point MUST be in temporal order.
///
/// \param data Input Data, may be pre-filtered, if so heed when interpreting the data
/// \param screen_pixel_size_cm Screen size in pixels. Note that this makes this function specific to the scene 
///								we're currently using.
/// \param estimation_function A function that estimates the point of gaze. See also point_jump_callback
/// \param point_jump_callback Since we're evaluating the accuracy on a specific set of points, the input data that 
///								is presented to us may not be the result of a continous recordnig. In particular, pauses
///								where data is omitted may lay inbetween the input data. In particular, the given input points
///								may also be temporally out of order.
CompleteCalibrationEvaluation evaluateAccuracy(const CalibrationData& data,
                                               const gazeestimation::Vec2& screen_pixel_size_cm,
                                               DetectionToGazeEstimation estimation_function,
												PointJumpCallback point_jump_callback,
												CalibrationEvaluationMode mode);


void calibrateGazeEstimationFrom(gazeestimation::EyeAndCameraParameters& setup_parameters, const CalibrationData& data,
                                 RemoteEyeGazeEstimation* gaze_estimation, EyePosition eye,
                                 const gazeestimation::Vec2& screen_pixel_size, const gazeestimation::Vec3& wcs_offset,
                                 const double z_shift);

#endif
