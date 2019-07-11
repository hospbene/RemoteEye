#ifndef GAZE_ESTIMATION_ADAPTERS_H_
#define GAZE_ESTIMATION_ADAPTERS_H_

#include "IntermediateTypes.h"
#include "../../../GazeEstimationCpp/GazeEstimationTypes.hpp"

gazeestimation::Vec3 calculatePointOfInterest(const gazeestimation::Vec3& cornea_center, const gazeestimation::Vec3& visual_axis_unit_vector, double z_shift);

gazeestimation::Vec2 estimateScreenPoint(const gazeestimation::Vec3& poi, double screen_pixel_size_x, double screen_pixel_size_y);

/// Adapter from the return value of the calibration to the format that the optimization backend dictates
/// for the variables. This enables the use of the code that applies a set of variables to the input parameters
/// to apply the final result as well.
///
/// This relies on the (de-)allocation behavior of the vector class. No memory is allocated,
/// and the returned pointers are only valid as long as no operation that changes the layout of any of the original
/// vectors is made.
const double* const* const vecvecToPointerPointer(std::vector<std::vector<double>>& a, std::vector<double*>& tmp);

/// Calibrates against alpha, beta, R, K, camera_rotation_y, camera_rotation_z
gazeestimation::EyeAndCameraParameters sixVariableCalibrationApplicator(gazeestimation::EyeAndCameraParameters params, double const* const* variables);

gazeestimation::Vec3 resultProcessor(const gazeestimation::DefaultGazeEstimationResult& result, double z_shift, const gazeestimation::Vec3& wcs_offset);


#endif