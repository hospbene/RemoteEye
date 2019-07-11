#ifndef DETECTOR_CODE_HPP_
#define DETECTOR_CODE_HPP_

#include "opencv2/core.hpp"

#include "IntermediateTypes.h"
#include "SparseInputImage.h"

struct DetectionWrapper;
class PupilDetection;
struct PupilGlintCombiInstance;

const unsigned int eye_region_size_x = 100;
const unsigned int eye_region_size_y = 100;



struct EyeFeatures
{
	bool is_valid = false;
	bool detected = false;
	// Ordered set of glints (corresponding in ordering to the light sources)
	std::vector<cv::Point2f> glints;
	// 
	cv::Point2f pupil_center;
};

std::vector<cv::Point2f> findGeometry(const std::vector<cv::Point2f>& glintCandidates, cv::Mat src);

// Removes all glints that are on sclera (surrounding brightness is less than 
// VAR: maxMeanAroundGlint)
std::vector<cv::Point2f> removeFalseGlints(std::vector<cv::Point2f> contourCenters, cv::Mat thresh, cv::Mat src);

//void GoalkeeperAnalysis::searchForGlints(Mat src, std::promise<vector<Point2f>> & p)
std::vector<cv::Point2f> searchForGlints(cv::Mat src, double firstEyeThresh);

/// \brief Calculates the center of mass of the glints
cv::Point2f calculateCenterOfMass(const std::vector<cv::Point2f>& glints);

/// \brief Creates a fixed size region around the given point.
cv::Rect eyeSubimageFromCoM(cv::Point2f center_of_mass);


bool cutEyeRegion(const cv::Mat& frame, cv::Mat& output, EyePosition desired_eye, double first_eye_threshold);

PupilGlintCombiInstance calcPupilGlintCombi(const cv::Point2f& pupilCenterRight,
	const cv::Point2f& pupilCenterLeft, std::vector<cv::Point2f> glintsLeft,
	std::vector<cv::Point2f> glintsRight);

/// Detects the glints and pupil from a region that contains only a single eye.
EyeFeatures detectGlintAndPupilSingle(const cv::Mat& frame, const cv::Rect& region, DetectionWrapper pupil_detection,
	double firstEyeThresh, bool precut = false);

PupilGlintCombiInstance detectGlintAndPupilNEW(const cv::Mat& frame, double firstEyeThresh, PupilDetection* pupil_detection);


PupilGlintCombiInstance detectGlintAndPupilNEWSparse(const SparseInputImage& frame, double firstEyeThresh, PupilDetection* pupil_detection);




#endif