#ifndef GAZE_ESTIMATION_TYPES_HPP_INCLUDED
#define GAZE_ESTIMATION_TYPES_HPP_INCLUDED

#define CERES_MSVC_USE_UNDERSCORE_PREFIXED_BESSEL_FUNCTIONS

#include "MathTypes.hpp"

#include <vector>

#include "PinholeCameraModel.hpp"

namespace gazeestimation{
	class PinholeCameraModel;

class DefaultGazeEstimationResult
{
public:
	bool is_valid;
	bool is_error;
	Vec3 center_of_cornea;
	Vec3 visual_axis;
	Vec3 optical_axis;

	std::string error;

	explicit DefaultGazeEstimationResult();

	static DefaultGazeEstimationResult make_error(std::string error);
};


template <class Parameters, class InputData, class GazeEstimationResult>
class GazeEstimationMethod
{
public:
	virtual ~GazeEstimationMethod();
	virtual GazeEstimationResult estimate(const InputData& data, const Parameters& parameters) = 0;
};



template <class CalibratedParameters>
class CalibrationMethod
{
public:
	virtual ~CalibrationMethod();
};

template <class Parameters, class InputData, class GazeEstimationResult>
GazeEstimationMethod<Parameters, InputData, GazeEstimationResult>::~GazeEstimationMethod() {}

template <class CalibratedParameters>
CalibrationMethod<CalibratedParameters>::~CalibrationMethod() {}

struct PupilCenterGlintInput
{
	Vec2 pupil_center;
	std::vector<Vec2> glints;	
};

struct PupilCenterGlintInputs
{
	std::vector<PupilCenterGlintInput> data;
};

struct EyeAndCameraParameters
{
	// eye parameters
	double alpha;
	double beta;
	double R; // R in cm
	double K; // K in cm
	double n1;
	double n2;
	double D; // D in cm

	std::vector<PinholeCameraModel> cameras;

	// lights
	std::vector<Vec3> light_positions; // light positions (ordered!, this is important for glint association)

	double distance_to_camera_estimate; // initial guess for the eye-camera distance
};

}

#endif