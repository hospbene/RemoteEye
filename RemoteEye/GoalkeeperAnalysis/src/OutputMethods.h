#ifndef OUTPUT_METHODS_H_
#define OUTPUT_METHODS_H_

#include "IntermediateTypes.h"
#include <opencv2/videoio.hpp>

class PupilDetection;

bool writeUserVideoFile(const std::string& filename, const VideoFrame2Timestamp& userVideoAssociation);
/// Draws the given gaze data on the provided video into the location provided.
/// \note This overload constructs the video writer with the applications default lossy codec.
bool drawGaze(const std::string& source_video, const std::string& output_file,
	const VideoFrame2Timestamp& stimulus_meta, const VideoFrame2Timestamp& eye_meta,
	const VideoFrame2Gaze& gaze_meta, const cv::Rect& original_stimulus_position);
void drawGaze(cv::VideoCapture* stimulus, cv::VideoWriter* output_writer,
	const VideoFrame2Timestamp& stimulus_meta,
	const VideoFrame2Timestamp& eye_meta,
	const VideoFrame2Gaze& gaze_meta, const cv::Rect& original_stimulus_position);

VideoFrame2Gaze estimateAndSaveEyeVideoFile(const std::string& eye_video, const std::string& output_filename,
	const VideoFrame2Timestamp& eyeVideoDictionary,
	DetectionToGazeEstimation estimation,
	PupilDetection* pupil_detection,
	int first_eye_threshold);

void drawFeatures(cv::Mat& frame, const DetectedFeatures& instance);

/// \brief Draws only the features of the given eye.
void drawFeatures(cv::Mat& frame, const DetectedFeatures& features, EyePosition eye);



#endif