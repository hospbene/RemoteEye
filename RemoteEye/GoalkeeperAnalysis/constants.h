#ifndef CONSTANTS_H_
#define CONSTANTS_H_
#pragma once
#include <opencv2/videoio/videoio_c.h>

const int lossless_video_codec = CV_FOURCC('H', 'F', 'Y', 'U');
const int lossy_video_codec = CV_FOURCC('H', '2', '6', '4');
const int initial_first_eye_threshold = 190;		// 176
const int initial_gain = 0;	//255
const int intial_gamma = 220;	// 220
const int initial_hw_gain = 50;	//220
const int target_camera_fps = 500;
const double target_exposure = 1.9;

// needed for the evaluation of the calibration

// CHANGE W
// TODO: get right distance from screen to VP
const float true_screen_distance_mm = 640;		//700  670

const bool use_video_file_in_debug = false;

#define PROFILING_ENABLED
#ifdef PROFILING_ENABLED
// best set at approximately the fps of the used source
const size_t profilingMonitorMaxObjectsRetained = 30;
#endif


#define DETECTOR_INNER_ASYNC

#define BORE_TRAINING_SAVE_VIDEO
#ifdef BORE_TRAINING_SAVE_VIDEO
const std::string bore_traning_video_folder = "bore_training_video";
#endif


enum class AggregationMode
{
	AVERAGE = 0,
	MEDIAN
};

//TODO: move all these settings somewhere else
const AggregationMode calibration_data_aggregation_mode = AggregationMode::MEDIAN;


const bool use_chen_noise_reduction = true;
const bool use_noise_reduction_filter = true;
const unsigned int noise_reduction_filter_length = 20;
const bool use_lowpass_filter = true;

#endif
