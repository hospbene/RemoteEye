#include "Calibration.h"

#include <fstream>

#include "constants.h"

#include "../../../GazeEstimationCpp/GazeEstimationTypes.hpp"
#include "../../../GazeEstimationCpp/GenericCalibration.hpp"

#include "GazeEstimationAdapters.hpp"
#include "utils.h"

#include "GoalkeeperAnalysis.h"
#include <opencv2/imgcodecs.hpp>
#include "DetectorCode.h"
#include <QMessageBox>

using namespace gazeestimation;


CompleteCalibrationEvaluation::CompleteCalibrationEvaluation(AggregatedCalibrationData aggregate_right,
                                                             AggregatedCalibrationData aggregate_left,
                                                             const CalibrationData& raw_data): 
aggregate_right(std::move(aggregate_right)),
aggregate_left(std::move(aggregate_left)),
raw_data(raw_data)
{
	
}

bool CompleteCalibrationEvaluation::writeToFile(const std::string& filename)
{
	std::ofstream calibrationEvaluationFile(filename);
	if (!calibrationEvaluationFile)
	{
		qWarning("Couldn't create calibration evaluation file.");
		return false;
	}
	
	// Print all found pupil-glint combinations in reference to the calibration point
	for (int i = 0; i < point_accuracies.size(); i++)
	{
		const auto& record = point_accuracies[i];
		
		calibrationEvaluationFile << "-------" << std::endl;
		calibrationEvaluationFile << "For Point " << i + 1 << std::endl;

		calibrationEvaluationFile << "Point on screen: " << record.centerOfCrossOnScreen << std::endl;
		calibrationEvaluationFile << "Avg estim point: " << record.averageEstimate << std::endl;
		//cv::Point2f difference = record.averageEstimate - record.centerOfCrossOnScreen;
		double distInPixel = sqrt(pow((record.averageEstimate.x - record.centerOfCrossOnScreen.x), 2) + pow((record.averageEstimate.y - record.centerOfCrossOnScreen.y), 2));

		//double distance = sqrt(difference.x * difference.x + difference.y * difference.y);
		double distance = distInPixel;

		calibrationEvaluationFile << "Dist Pixels    : " << distance << std::endl;
		calibrationEvaluationFile << "Precision(StdDev): " << record.averagePrecision << std::endl;
		calibrationEvaluationFile << "Accuracy (Mean): " << record.averageAccuracy << std::endl << std::endl << std::endl;

	}

	calibrationEvaluationFile << " Total Precision(StdDev) " << averagePrecision << std::endl;
	calibrationEvaluationFile << "Total Accuracy(MEAN) " << averageAccuracy << std::endl << "\n\n";;

	calibrationEvaluationFile.close();


	



	return true;
}

bool CompleteCalibrationEvaluation::writeImage(const std::string& filename)
{
	cv::Size image_size(1920, 1080);

	for(const auto& point : point_accuracies)
	{
		if (point.centerOfCrossOnScreen.x > image_size.width)
			image_size.width = point.centerOfCrossOnScreen.x;
		if (point.centerOfCrossOnScreen.y > image_size.height)
			image_size.height = point.centerOfCrossOnScreen.y;
	}

	image_size = cv::Size(image_size.width * 1.1, image_size.height * 1.1);

	const std::vector<cv::Scalar> colors = {
		cv::Scalar(200,0,0),
		cv::Scalar(0,200,0),
		cv::Scalar(0,0,200),
		cv::Scalar(200,0,200),
		cv::Scalar(0,200,200)
	};

	cv::Mat result(image_size, CV_8UC3);
	result = cv::Scalar(255, 255, 255);
	//cv::line(result, offset, cv::Point2f(offset.x + image_size.width, offset.y), cv::Scalar(0, 0, 0), 1);
	//cv::line(result, offset, cv::Point2f(offset.x, offset.y + image_size.height), cv::Scalar(0, 0, 0), 1);
	
	for (int i = 0; i < point_accuracies.size(); i++)
	{
		const auto& record = point_accuracies[i];
		const auto this_color = colors[i%colors.size()];
		const auto this_color_marker = colors[i%colors.size()]+cv::Scalar(40,40,40);
		cv::drawMarker(result, record.centerOfCrossOnScreen, this_color, cv::MARKER_CROSS, 20, 1);
		cv::circle(result, record.averageEstimate, 5, this_color_marker, -1);

		for(const auto& individual_point : record.allEstimates)
		{
			cv::circle(result, individual_point, 2, this_color, -1);
			
		}
	}

	return cv::imwrite(filename, result);
}

bool CompleteCalibrationEvaluation::writeInputEvluationImage(const std::string& filename, const std::string& glints_filename)
{
	int total_points = this->raw_data.calibration_points.size();
	int num_x_axis = sqrt(total_points);
	int num_y_axis = ceil(total_points / num_x_axis);
	cv::Mat pupil_image(cv::Size(num_x_axis * eye_region_size_x, num_y_axis * eye_region_size_y), CV_8UC1, cv::Scalar(0,0,0));
	cv::Mat glints_image(cv::Size(eye_region_size_x * SingleEyeFeatures::num_glints, total_points * eye_region_size_y), CV_8UC1, cv::Scalar(0,0,0));

	int idx = 0;
	for(const auto& point : raw_data.calibration_points)
	{
		int row = idx / num_x_axis;
		int col = idx % num_x_axis;
		cv::Point2f base = cv::Point2f(col * eye_region_size_x, row * eye_region_size_y);
		
		for(const auto& datapoint : point.data)
		{
			if (!datapoint.right_eye.pupilValid())
				continue;

			cv::Point2f coords = base + datapoint.right_eye.pupil_center();

			if (coords.x < 0 || coords.y < 0)
				continue;
			if (coords.x >= pupil_image.cols)
				coords.x = pupil_image.cols - 1;
			if (coords.y >= pupil_image.rows)
				coords.y = pupil_image.rows - 1;


			unsigned char src_point = pupil_image.at<unsigned char>(coords);
			if (src_point < 245)
				src_point += 1;
			pupil_image.at<unsigned char>(coords) = src_point;

			for(size_t i = 0; i < SingleEyeFeatures::num_glints; i++)
			{
				if (!datapoint.right_eye.glintValid(i))
					continue;
				cv::Point2f glint_base(i * eye_region_size_x, idx * eye_region_size_y);
				cv::Point2f glint = glint_base + datapoint.right_eye.glint(i);
				if (!glint.inside(cv::Rect(0, 0, glints_image.cols, glints_image.rows)))
					continue;

				unsigned char glint_val = glints_image.at<unsigned char>(glint);
				if (glint_val < 245)
					glint_val += 1;
				glints_image.at<unsigned char>(glint) = glint_val;
			}
		}
		idx++;
	}

	cv::Mat equalized_hist;
	cv::equalizeHist(pupil_image, equalized_hist);

	cv::Mat glints_equalized;
	cv::equalizeHist(glints_image, glints_equalized);

	cv::Mat out_image;
	cv::applyColorMap(equalized_hist, out_image, cv::COLORMAP_HOT);
	cv::imwrite(filename, out_image);

	cv::Mat out_glints;
	cv::applyColorMap(glints_equalized, out_glints, cv::COLORMAP_HOT);
	cv::imwrite(glints_filename, out_glints);

	return true;
}



double visualAngle(cv::Point2f point1, cv::Point2f point2, float screen_pixel_size_x_cm, float screen_pixel_size_y_cm, float true_screen_distance_mm)
{
// 1. Pixel size of screen
// mean value of both. they are more or less the same. Depends on accurate measurement.
double pixel_size_cm = (screen_pixel_size_x_cm + screen_pixel_size_y_cm) / 2;

// 2. Calculate euclidian distance from point 1 to point 2 in pixels
double distInPixel = sqrt(pow((point2.x - point1.x), 2) + pow((point2.y - point1.y), 2));

// 3. compute distance in pixels into distance in mm
double distInMM = distInPixel * pixel_size_cm * 10;

// 4. visual angle
double visualAngle = 2 * atan((distInMM / (2 * true_screen_distance_mm)));

double visualAngleInDegree = (visualAngle / (2 * M_PI)) * 360;

return visualAngleInDegree;
}


CalibrationAccuracyRecord calculateAccuracy(CalibrationAccuracyRecord record, const Vec2& screen_pixel_size_cm, float true_screen_distance_mm)
{
	std::vector<double> euclDistances;
	double summedDistances = 0;

	// 1. Print all estimated Points on screen
	for (int i = 0; i < record.allEstimates.size(); i++)
	{

		double dist = visualAngle(record.allEstimates[i], record.centerOfCrossOnScreen, screen_pixel_size_cm[0], screen_pixel_size_cm[1], true_screen_distance_mm);
		euclDistances.emplace_back(dist);

	}

	// 1.2 Accuracy: divide Summed Distances for one point by number of samples	=> Accuracy
	if (summedDistances >= 0 && !euclDistances.empty())
	{
		record.averageAccuracy = summedDistances / euclDistances.size();
	}

	return record;

}


void CompleteCalibrationEvaluation::writeEstimationsToFile(const std::string& filename)
{
	std::ofstream estimationsFile(filename);
	if (!estimationsFile)
	{
		qWarning("Couldn't create calibration evaluation file.");
		return;
	}

	//write header
	estimationsFile << "Screen Point" << ";" << "Avg. Estimation" << "; Sample Estimation" << std::endl;


	// 1. calibration_points.position
	for (int i = 0; i < this->raw_data.calibration_points.size(); i++)
	{
		// 2. loop over calibraiont_points.data = DetectedFeatures
		for (int a = 0; a < this->point_accuracies[i].allEstimates.size(); a++)
		{
			estimationsFile << this->raw_data.calibration_points[i].position << ";" << this->point_accuracies[i].averageEstimate << ";" << this->point_accuracies[i].allEstimates[a] << std::endl;
		}
	}
	estimationsFile.close();
}

void CompleteCalibrationEvaluation::writeRawData(const std::string& filename)
{
	std::ofstream rawDataFile(filename);
	if (!rawDataFile)
	{
		qWarning("Couldn't create calibration evaluation file.");
		return;
	}

	//write header
	rawDataFile << "Screen Point x;" << "Screen Point y;" 
		<< "Pupil_right x;" << "Pupil_right y;"
		<< "Pupil_left x;" << "Pupil_left y;" 
		<< "Glint_right_1 x;" << "Glint_right_1 y;"
		<< "Glint_right_2 x;" << "Glint_right_2 y;"
		<< "Glint_right_3 x;" << "Glint_right_3 y;"
		<< "Glint_left_1 x;" << "Glint_left_1 y;"
		<< "Glint_left_2 x;" << "Glint_left_2 y;"
		<< "Glint_left_3 x;" << "Glint_left_3 y;"
		<< std::endl;


	// 1. calibration_points.position
	for (int i = 0; i < this->raw_data.calibration_points.size(); i++)
	{
		// 2. loop over calibraiont_points.data = DetectedFeatures
		for (int a = 0; a < this->raw_data.calibration_points[i].data.size(); a++)
		{
			rawDataFile << this->raw_data.calibration_points[i].position[0] << ";" 
						<< this->raw_data.calibration_points[i].position[1] << ";"
						<< this->raw_data.calibration_points[i].data[a].right_eye.pupil_center().x << ";"
						<< this->raw_data.calibration_points[i].data[a].right_eye.pupil_center().y << ";"
						<< this->raw_data.calibration_points[i].data[a].left_eye.pupil_center().x << ";"
						<< this->raw_data.calibration_points[i].data[a].left_eye.pupil_center().y << ";"
						<< this->raw_data.calibration_points[i].data[a].right_eye.glint(0).x << ";" 
						<< this->raw_data.calibration_points[i].data[a].right_eye.glint(0).y << ";"
						<< this->raw_data.calibration_points[i].data[a].right_eye.glint(1).x << ";" 
						<< this->raw_data.calibration_points[i].data[a].right_eye.glint(1).y << ";"
						<< this->raw_data.calibration_points[i].data[a].right_eye.glint(2).x << ";"
						<< this->raw_data.calibration_points[i].data[a].right_eye.glint(2).y << ";"
						<< this->raw_data.calibration_points[i].data[a].left_eye.glint(0).x << ";" 
						<< this->raw_data.calibration_points[i].data[a].left_eye.glint(0).y << ";"
						<< this->raw_data.calibration_points[i].data[a].left_eye.glint(1).x << ";" 
						<< this->raw_data.calibration_points[i].data[a].left_eye.glint(1).y << ";"
						<< this->raw_data.calibration_points[i].data[a].left_eye.glint(2).x << ";"
						<< this->raw_data.calibration_points[i].data[a].left_eye.glint(2).y << ";"
						<< std::endl;
		}
	}
	rawDataFile.close();
}

void CompleteCalibrationEvaluation::writeFeatureDetectionRate(const std::string& filename)
{
	std::ofstream detectionRateFile(filename);
	if (!detectionRateFile)
	{
		qWarning("Couldn't create calibration evaluation file.");
		return;
	}

	//write header
	detectionRateFile << "Total frames: " << this->totalFrameCount << std::endl
		<< "Combined Detected: " << this->totalDetectionRate << std::endl
		<< "Combined Not detected: " << this->totalUnDetectionRate << std::endl
		<< "Percentage total: " << this->percentageDetection << std::endl << std::endl
		<< "Detected left: " << this->totalDetectionRateLEFT << std::endl
		<< "Not detected left: "<< this->totalUnDetectionRateLEFT << std::endl
		<< "Percentage Left: " << this->percentageDetectionLeft << std::endl << std::endl
		<< "Detected right: " << this->totalDetectionRateRIGHT << std::endl
		<< "Not detected right: " << this->totalUnDetectionRateRIGHT << std::endl
		<< "Percentage Right: " << this->percentageDetectionRight << std::endl;

		detectionRateFile.close();
}

CompleteCalibrationEvaluation evaluateAccuracy(const CalibrationData& data,
	const gazeestimation::Vec2& screen_pixel_size_cm, DetectionToGazeEstimation estimation_function,
	PointJumpCallback point_jump_callback, CalibrationEvaluationMode mode)
{
	AggregatedCalibrationData aggregate_right(data, EyePosition::RIGHT, calibration_data_aggregation_mode);
	AggregatedCalibrationData aggregate_left(data, EyePosition::LEFT, calibration_data_aggregation_mode);

	CompleteCalibrationEvaluation result(aggregate_right, aggregate_left, data);

	int validityCounter = 0;

	std::vector<double> visualAngles;
	int mostPoints;
	int totalFrames;



	switch (mode)
	{
		case CalibrationEvaluationMode::LEFT_ONLY:
			 mostPoints = aggregate_left.points.size();
			break;

		case CalibrationEvaluationMode::RIGHT_ONLY:
			mostPoints = aggregate_right.points.size();
			break;

		case CalibrationEvaluationMode::BOTH:
			mostPoints = (aggregate_left.points.size() > aggregate_right.points.size()) ? aggregate_left.points.size() : aggregate_right.points.size();
			break;
		default:
			mostPoints = (aggregate_left.points.size() > aggregate_right.points.size()) ? aggregate_left.points.size() : aggregate_right.points.size();
			break;

	}


	std::vector<CalibrationAccuracyRecord> calibrationPoints;
	// Print all found pupil-glint combinations in reference to the calibration point
	//for (int i = 0; i < mostPoints; i++)
	for (int i = 0; i < mostPoints; i++)
	{
		const auto& point_right = aggregate_right.points[i];
		const auto& point_left = aggregate_left.points[i];

		CalibrationAccuracyRecord record(cv::Point2f(aggregate_right.points[i].point_on_screen[0], aggregate_right.points[i].point_on_screen[1]));

		record.averagePrecision = 0;
		record.averageAccuracy = 0;

		DetectedFeatures features;
		features.left_eye = point_left.aggregate_input;
		features.right_eye = point_right.aggregate_input;

		auto estimated = estimation_function(features);
		record.averageEstimate = cv::Point2f(estimated.position.x, estimated.position.y);

		if (mode == CalibrationEvaluationMode::RIGHT_ONLY || mode == CalibrationEvaluationMode::LEFT_ONLY)
		{
			const auto& point_source = mode == CalibrationEvaluationMode::RIGHT_ONLY ? point_right : point_left;
			const EyePosition eye = mode == CalibrationEvaluationMode::RIGHT_ONLY ? EyePosition::RIGHT : EyePosition::LEFT;
			for (const auto& subpoint : point_source.raw_data)
			{
				if (!subpoint.eye(eye).allValid())
					continue;

				DetectedFeatures features;
				if (eye == EyePosition::RIGHT)
					features.right_eye = subpoint.eye(eye);
				else
					features.left_eye = subpoint.eye(eye);
				auto inner_estimated = estimation_function(features);
				record.allEstimates.push_back(cv::Point2f(inner_estimated.position.x, inner_estimated.position.y));
			}

			
			// Count undetected features
			// count glints note detected
			// when glints are not detected, there is no image for pupil => whole eye is not detected too

			if (mode == CalibrationEvaluationMode::RIGHT_ONLY)
			{
				if (features.right_eye.glintValid(0) && features.right_eye.glintValid(1) && features.right_eye.glintValid(2) && features.right_eye.pupilValid())
				{
					result.totalDetectionRateRIGHT++;
				}
				else
				{
					result.totalUnDetectionRateRIGHT++;
				}
			}

			if (mode == CalibrationEvaluationMode::LEFT_ONLY)
			{
				if (features.left_eye.glintValid(0) && features.left_eye.glintValid(1) && features.left_eye.glintValid(2) && features.left_eye.pupilValid())
				{
					result.totalDetectionRateLEFT++;
				}
				else
				{
					result.totalUnDetectionRateLEFT++;
				}
			}
		}

		if (mode == CalibrationEvaluationMode::BOTH)
		{
			const auto& point_source_right = point_right;
			const auto& point_source_left = point_left;


			for (int a=0; a < point_source_right.raw_data.size();a++)
			{
				// right eye is valid
				if (point_source_right.raw_data[a].eye(EyePosition::RIGHT).allValid())
				{
					// left eye is valid
					if (point_source_right.raw_data[a].eye(EyePosition::LEFT).allValid())
					{

						DetectedFeatures features;
						features.right_eye = point_source_right.raw_data[a].eye(EyePosition::RIGHT);
						auto inner_estimated_right = estimation_function(features);

						DetectedFeatures features2;
						features2.left_eye = point_source_left.raw_data[a].eye(EyePosition::LEFT);
						auto inner_estimated_left = estimation_function(features2);

						ScreenspaceGazeResult both((inner_estimated_right.position + inner_estimated_left.position) / 2);
						record.allEstimates.push_back(cv::Point2f(both.position.x, both.position.y));
					}	// only right eye is valid. take right eye estimation
					else
					{
						DetectedFeatures features;
						features.right_eye = point_source_right.raw_data[a].eye(EyePosition::RIGHT);
						auto inner_estimated_right = estimation_function(features);
						record.allEstimates.push_back(cv::Point2f(inner_estimated_right.position.x, inner_estimated_right.position.y));

					}
				} // right eye is not valid
				else
				{
					if (point_source_right.raw_data[a].eye(EyePosition::LEFT).allValid())
					{
						DetectedFeatures features;
						features.left_eye = point_source_right.raw_data[a].eye(EyePosition::LEFT);
						auto inner_estimated_left = estimation_function(features);
						record.allEstimates.push_back(cv::Point2f(inner_estimated_left.position.x, inner_estimated_left.position.y));
					}
					else
					{
						continue;
					}
				}	
				
				
				
					if (features.right_eye.glintValid(0) && features.right_eye.glintValid(1) && features.right_eye.glintValid(2) && features.right_eye.pupilValid())
					{
						result.totalDetectionRateRIGHT++;
					}
					else
					{
						result.totalUnDetectionRateRIGHT++;
					}
				
				
					if (features.left_eye.glintValid(0) && features.left_eye.glintValid(1) && features.left_eye.glintValid(2) && features.left_eye.pupilValid())
					{
						result.totalDetectionRateLEFT++;
					}
					else
					{
						result.totalUnDetectionRateLEFT++;
					}
			}
		}

		// =========================== START DETECTED / UNDETECTED COUNT
		if (result.totalUnDetectionRateRIGHT > 0)
		{
			result.percentageDetectionRight = (result.totalDetectionRateRIGHT / result.totalFrameCount) * 100;
		}
		else
		{
			result.percentageDetectionRight = 100.0;
		}

		if (result.totalUnDetectionRateLEFT > 0)
		{
			result.percentageDetectionLeft = (result.totalDetectionRateLEFT / result.totalFrameCount) * 100;
		}
		else
		{
			result.percentageDetectionLeft = 100.0;
		}
		
		result.totalDetectionRate = (result.totalDetectionRateLEFT + result.totalDetectionRateRIGHT) /2;
		result.totalUnDetectionRate = (result.totalUnDetectionRateLEFT + result.totalUnDetectionRateRIGHT)/2;
		result.totalFrameCount = result.totalDetectionRate + result.totalUnDetectionRate;

		if (result.totalDetectionRate != 0)
			result.percentageDetection = (result.totalDetectionRate / result.totalFrameCount) * 100;

		// =========================== END DETECTED / UNDETECTED COUNT

				
		for (int j = 0; j < record.allEstimates.size(); j++)
		{
			double dist = visualAngle(record.allEstimates[j], record.centerOfCrossOnScreen, screen_pixel_size_cm[0], screen_pixel_size_cm[1], true_screen_distance_mm);

			// 1. euclidian Distances between sample and calibration point
			if(dist > 0.00)
				visualAngles.emplace_back(dist);
		}
		

		// 3.1 opencv: calculate mean values of all distances => Accuracy MEAN
		// and also calculate the standard deviation of all distances => Precision
		if (!visualAngles.empty())
		{
			cv::Scalar mean, stdDev;
			cv::meanStdDev(visualAngles, mean, stdDev);

			//record.meanAccuracy = mean[0];
			record.averageAccuracy = visualAngle(record.averageEstimate, record.centerOfCrossOnScreen, screen_pixel_size_cm[0], screen_pixel_size_cm[1], true_screen_distance_mm);
			record.averagePrecision = stdDev[0];
			result.averageAccuracy += record.averageAccuracy;
			result.averagePrecision += record.averagePrecision;
			validityCounter++;
		}

		result.point_accuracies.push_back(record);

		visualAngles.clear();

		point_jump_callback();
	}


	result.averageAccuracy /= validityCounter;
	result.averagePrecision /= validityCounter;

	return result;
}




/*
CompleteCalibrationEvaluation evaluateAccuracy(const CalibrationData& data,
	const gazeestimation::Vec2& screen_pixel_size_cm, DetectionToGazeEstimation estimation_function,
	PointJumpCallback point_jump_callback, CalibrationEvaluationMode mode)
{
	AggregatedCalibrationData aggregate_right(data, EyePosition::RIGHT, calibration_data_aggregation_mode);
	AggregatedCalibrationData aggregate_left(data, EyePosition::LEFT, calibration_data_aggregation_mode);
	CompleteCalibrationEvaluation result(aggregate_right, aggregate_left, data);
	
	int validityCounterPrecision = 0;
	int validityCounterAccuracy = 0;
	int validityCounterstdDev = 0;

	std::vector<double> visualAngles;
	std::vector<double> euclDistances;


	std::vector<CalibrationAccuracyRecord> calibrationPoints;


	// Print all found pupil-glint combinations in reference to the calibration point
	for (int i = 0; i < aggregate_right.points.size(); i++)
	{
		const auto& point_right = aggregate_right.points[i];
		const auto& point_left = aggregate_left.points[i];

		CalibrationAccuracyRecord record(cv::Point2f(aggregate_right.points[i].point_on_screen[0], aggregate_right.points[i].point_on_screen[1]));

		record.averagePrecision = 0;
		record.averageAccuracy = 0;

		DetectedFeatures features;
		features.left_eye = point_left.aggregate_input;
		features.right_eye = point_right.aggregate_input;

		// estimated = gaze estimation auf aggregierte inputs
		auto estimated = estimation_function(features);
					//record.averageEstimate = cv::Point2f(estimated.position.x, estimated.position.y);

		if(mode == CalibrationEvaluationMode::RIGHT_ONLY || mode == CalibrationEvaluationMode::LEFT_ONLY)
		{
			const auto& point_source = mode == CalibrationEvaluationMode::RIGHT_ONLY ? point_right : point_left;
			const EyePosition eye = mode == CalibrationEvaluationMode::RIGHT_ONLY ? EyePosition::RIGHT : EyePosition::LEFT;
			for(const auto& subpoint : point_source.raw_data)
			{
				if (!subpoint.eye(eye).allValid())
					continue;

				DetectedFeatures features;
				if(eye == EyePosition::RIGHT)
					features.right_eye = subpoint.eye(eye);
				else
					features.left_eye = subpoint.eye(eye);
				auto inner_estimated = estimation_function(features);
				record.allEstimates.push_back(cv::Point2f(inner_estimated.position.x, inner_estimated.position.y));
				//qDebug() << "Number of samples: " << record.allEstimates.size();
			}
		}

		qDebug() << "Evaluating calibration point " << i;
		qDebug() << "CalibrationPoint: " << record.centerOfCrossOnScreen;
		qDebug() << "Number of samples: " << record.allEstimates.size();
		calculateAccuracy(record, screen_pixel_size_cm, true_screen_distance_mm);
		*/


// 1. Calculate accuracy
//////////////////////////////////////////////////////////////////////////

		/*
		std::vector<double> euclDistances;
		double summedDistances = 0;

		// 1. Print all estimated Points on screen
		for (int i = 0; i < record.allEstimates.size(); i++)
		{
			//qDebug() << "Estimated Point: " << record.allEstimates[i];

			// visual Angle zwische calibration point und sample
			double dist = visualAngle(record.allEstimates[i], record.centerOfCrossOnScreen, screen_pixel_size_cm[0], screen_pixel_size_cm[1], true_screen_distance_mm);
			//qDebug() << "Euclidian Distance to Calibration Point: " << dist << "\n";

			// all euclidische Distanzen sammeln
			euclDistances.emplace_back(dist);
			summedDistances += dist;
		}


		double averageAccuracy = 0;
		// 1.2 Accuracy: divide Summed Distances for one point by number of samples	=> Accuracy
		if (summedDistances >= 0 && !euclDistances.empty())
		{
			averageAccuracy = summedDistances / euclDistances.size();
			result.meanAccuracy += record.averageAccuracy;
			validityCounterAccuracy++;
		}

		record.averageAccuracy = averageAccuracy;
		qDebug() << "Accuracy: " << averageAccuracy;

		euclDistances.clear();*/

		// Accuracy END
///////////////////////////////////////////////////////////////////////////


		//calculatePrecisionRMS();
		//calculatePrecisionSTDDev();


		/*
		// 3.1 opencv: calculate mean values of all distances => Accuracy MEAN
		// and also calculate the standard deviation of all distances
		if (!euclDistances.empty())
		{
			cv::Scalar mean, stdDev;
			cv::meanStdDev(euclDistances, mean, stdDev);

			record.meanAccuracy = mean[0];
			record.stdPrec = stdDev[0];
			result.meanSTDPrecision += record.stdPrec;
			result.meanMEANAccuracy += record.meanAccuracy;
			validityCounterstdDev++;
		}

		result.point_accuracies.push_back(record);

		

		visualAngles.clear();

		point_jump_callback();
	}


	result.meanPrecision /= validityCounterPrecision;
	result.meanAccuracy /= validityCounterAccuracy;

	result.meanSTDPrecision /= validityCounterstdDev;
	result.meanMEANAccuracy /= validityCounterstdDev;
	
	return result;

	
}
*/

void calculatePrecisionRMS()
{
	/*
	// 2.1 Sum all squared values of all visual Angles from consecutive samples
	double summed = 0;
	for (int a = 0; a < visualAngles.size(); a++)
	{
		summed += pow(visualAngles[a], 2);
		qDebug() << "2. Summed squared visual angles" << summed;
	}


	// 2.1. Get all summed squared visual angles of consecutive samples and divide them by the number of samples
	// take root of it
	if (summed >= 0 && !visualAngles.empty())
	{
		record.averagePrecision = sqrt(summed / visualAngles.size());
		result.meanPrecision += record.averagePrecision;
		validityCounterPrecision++;
	}
	*/
}

void calculatePrecisionSTDDev()
{

}

void calibrateGazeEstimationFrom(EyeAndCameraParameters& setup_parameters, const CalibrationData& data,
	RemoteEyeGazeEstimation* gaze_estimation, EyePosition eye,
	const Vec2& screen_pixel_size, const Vec3& wcs_offset, const double z_shift)
{
	AggregatedCalibrationData calibration_data(data, eye, calibration_data_aggregation_mode);

	// calibrate from it
	GenericCalibration<EyeAndCameraParameters, PupilCenterGlintInputs, DefaultGazeEstimationResult> calibration;
	
	/// change the true positions into world coordinate system so as to not need to carry
	/// the dependent data for that calculation into the calibration process
	std::vector<std::pair<PupilCenterGlintInputs, Vec3>> calibrate_against;
	for (auto It = calibration_data.points.begin(); It != calibration_data.points.end(); ++It)
	{
		// don't calibrate where there is not enough input
		if (It->raw_data.empty())
			continue;

		if (!It->aggregate_input.allValid())
			continue;

		const Vec2 true_pog = (*It).point_on_screen;
		Vec3 pog_wcs = make_vec3(true_pog[0] * screen_pixel_size[0], -true_pog[1] * screen_pixel_size[1], 0);
		PupilCenterGlintInputs inputs;
		inputs.data = { static_cast<PupilCenterGlintInput>((*It).aggregate_input) };
		qDebug() << inputs.data[0].pupil_center[0];
		calibrate_against.push_back(std::make_pair(inputs, pog_wcs));
	}

	if(calibrate_against.empty())
	{
		QMessageBox box;
		box.setText("Could not calibrate one eye because not enough valid samples were present.");
		box.setStandardButtons(QMessageBox::Cancel);
		box.exec();
		return;
	}

	std::vector<std::vector<double>> initial_values = {
		{ setup_parameters.alpha },
		{ setup_parameters.beta },
		{ setup_parameters.R },
		{ setup_parameters.K },
		{ setup_parameters.cameras[0].camera_angle_y() },
		{ setup_parameters.cameras[0].camera_angle_z() } };
	std::vector<std::vector<std::pair<double, double>>> bounds = {
		{ std::make_pair(deg_to_rad(-10), deg_to_rad(10)) },
		{ std::make_pair(deg_to_rad(-20), deg_to_rad(20)) },
		{ std::make_pair(0.3, 1.0) },
		{ std::make_pair(0.2, 1.0) },
		{ std::make_pair(deg_to_rad(-50), deg_to_rad(50)) },
		{ std::make_pair(deg_to_rad(-10), deg_to_rad(10)) }
	};

	auto calibration_result = calibration.calibrate(*gaze_estimation, setup_parameters,
		sixVariableCalibrationApplicator,
		std::bind(resultProcessor, std::placeholders::_1, z_shift, wcs_offset),
		calibrate_against,
		initial_values,
		bounds);
	std::vector<double*> tmp;
	setup_parameters = sixVariableCalibrationApplicator(setup_parameters, vecvecToPointerPointer(calibration_result, tmp));

	//
	qDebug() << "Calibration result: ";
	qDebug() << "Alpha: " << setup_parameters.alpha << " (" << rad_to_deg(setup_parameters.alpha) << ")" << "\n";
	qDebug() << "Beta: " << setup_parameters.beta << " (" << rad_to_deg(setup_parameters.beta) << ")" << "\n";
	qDebug() << "R: " << setup_parameters.R << "\\n";
	qDebug() << "K: " << setup_parameters.K << "\\n";
	qDebug() << "CamAy: " << setup_parameters.cameras[0].camera_angle_y() << "\n";
	qDebug() << "CamAz: " << setup_parameters.cameras[0].camera_angle_z() << "\n";
}

