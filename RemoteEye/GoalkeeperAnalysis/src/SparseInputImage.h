#ifndef SPARSE_INPUT_IMAGE_H_
#define SPARSE_INPUT_IMAGE_H_

#include <opencv2/core/mat.hpp>
#include "IntermediateTypes.h"

/// An input image where the main image parts have already been separated out to save data for storage
struct SparseInputImage
{
	cv::Mat left_eye;
	cv::Mat right_eye;
};

struct QueuedSparseInput
{
	SparseInputImage image;
	FrameTimestamp origin_time;
};

#endif