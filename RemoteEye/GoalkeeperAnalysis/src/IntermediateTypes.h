#ifndef INTERMEDIATE_TYPES_H_
#define INTERMEDIATE_TYPES_H_

#include <opencv2/core.hpp>
#include "../../../GazeEstimationCpp/GazeEstimationTypes.hpp"
#include <map>


// forward declarations for some of the gaze estimation types
namespace gazeestimation{
template <class Parameters, class InputData, class GazeEstimationResult>
class GazeEstimationMethod;
struct EyeAndCameraParameters;
struct PupilCenterGlintInputs;
class DefaultGazeEstimationResult;
}

// the type of gaze estimation method this project uses
typedef gazeestimation::GazeEstimationMethod<gazeestimation::EyeAndCameraParameters, 
											 gazeestimation::PupilCenterGlintInputs, 
											 gazeestimation::DefaultGazeEstimationResult> RemoteEyeGazeEstimation;

typedef __int64 FrameTimestamp;

struct PupilGlintCombiInstance;


enum class EyePosition
{
	RIGHT = 0,
	LEFT
};

struct ScreenspaceGazeResult
{
	explicit ScreenspaceGazeResult();
	explicit ScreenspaceGazeResult(cv::Point2f position);

	const bool valid;
	/// Note that a negative position here need not indicate that the detection is invalid, but simply
	/// that the subject looked to the left of the screen. Only the valid-flag indicates whether or not
	/// the result is valid.
	const cv::Point2f position;
};

struct EndToEndGazeEstimationInput
{
	std::vector<cv::Mat*> images;
};

// This represents the same features as the gazeestimation::PupilCenterGlintInputs, but is independent of the gaze estimation backend
// Glints are ordered (corresponding to the light sources).
//
class SingleEyeFeatures
{
	// Since a detected glint/pupil can never have been detected at a negative position,
	// this class internally uses any position with any coordinate < 0 to signify
	// that the value is not valid.
public:
	const static size_t num_glints = 3;

	SingleEyeFeatures();
	SingleEyeFeatures(const SingleEyeFeatures&) = default;
	/// Constructs this from data. This sets the instance valid flag to true.
	/// The number of glints must be equal to num_glints, otherwise an exception is raised.
	explicit SingleEyeFeatures(cv::Point2f pupil_center, std::vector<cv::Point2f> glints);

	explicit operator gazeestimation::PupilCenterGlintInput() const;
	operator bool() const;

	inline cv::Point2f pupil_center() const
	{
		return pupil_center_;
	}

	/// Returns the glint for the given index. Makes no guarantee the resultant glint is valid.
	cv::Point2f glint(int index) const;
	bool glintValid(int index) const;
	bool pupilValid() const;
	bool allValid() const;


private:
	bool valid = false;
	std::vector<cv::Point2f> glints;
	cv::Point2f pupil_center_;
};


struct DetectedFeatures
{
	explicit DetectedFeatures() = default;
	explicit DetectedFeatures(const PupilGlintCombiInstance& source);

	SingleEyeFeatures left_eye;
	SingleEyeFeatures right_eye;
	
	SingleEyeFeatures eye(EyePosition position) const;
};

// auxiliary typedefs
/// For the purposes here, the total pipeline is
/// is a function from a set of video image inputs to the coordinates of the gaze on screen. 
/// This further wraps the process so that the parameters for the setup and detector code do not
/// need to be carried through all the processing functions.
typedef std::function<ScreenspaceGazeResult(const EndToEndGazeEstimationInput&)> EndToEndGazeEstimation;
typedef std::function<ScreenspaceGazeResult(const DetectedFeatures&)> DetectionToGazeEstimation;


struct CalibrationPointData
{
	// the true position on screen.
	cv::Vec2f position;
	std::vector<DetectedFeatures> data;

	// The start of calibration point display
	FrameTimestamp start_time;
	// The end of calibration point display
	FrameTimestamp end_time;
};

/// Contains the raw data gathered during the calibration process
struct CalibrationData
{
	CalibrationData() = default;
	CalibrationData(const CalibrationData& other) = default;
	~CalibrationData() = default;
	std::vector<CalibrationPointData> calibration_points;
};



enum class CalibrationEvaluationMode
{
	LEFT_ONLY,
	RIGHT_ONLY,
	BOTH
};


struct QueuedFrame
{
	cv::Mat frame;
	FrameTimestamp origin_time;
};

struct VideoFrame2Timestamp
{
	FrameTimestamp start_time;
	FrameTimestamp end_time;
	std::map<unsigned long, double> frame2time;
};

struct VideoFrame2Gaze
{
	FrameTimestamp start_time;
	FrameTimestamp end_time;
	std::map<unsigned long, cv::Point2f> frame2gaze;
};


// one instance of a combination of glints and pupil center
// a lot of these are saved in a temp structure allPoint2fsFoundOnX
// a temp Strcuture contains all found positions during one calibration Point2f
struct PupilGlintCombiInstance
{
	bool valid = true;
	cv::Point2f pupilLeft = { 0, 0 };
	cv::Point2f pupilRight = { 0, 0 };
	cv::Point2f glintLeft_1 = { 0, 0 };
	cv::Point2f glintLeft_2 = { 0, 0 };
	cv::Point2f glintLeft_3 = { 0, 0 };
	cv::Point2f glintRight_1 = { 0, 0 };
	cv::Point2f glintRight_2 = { 0, 0 };
	cv::Point2f glintRight_3 = { 0, 0 };
	bool detectedLeft = true;
	bool detectedRight = true;


	void log() const;

	void toFile(std::ostream* file) const;

	void clear()
	{
		valid = true;
		pupilLeft = { 0, 0 };
		pupilRight = { 0, 0 };
		glintLeft_1 = { 0, 0 };
		glintLeft_2 = { 0, 0 };
		glintLeft_3 = { 0, 0 };
		glintRight_1 = { 0, 0 };
		glintRight_2 = { 0, 0 };
		glintRight_3 = { 0, 0 };
	}
};


// both of these are the currently used debugging builds
#if defined_DEBUG || defined REL_WITH_DEBUG_INFO
#define GA_ASSERT(expression) if(!(expression)) { throw std::exception("Assertion failed: #expression#");}
#else
#define GA_ASSERT(expression)
#endif

#endif
