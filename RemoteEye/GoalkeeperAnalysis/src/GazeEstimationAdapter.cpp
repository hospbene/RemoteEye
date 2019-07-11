#include "GazeEstimationAdapters.hpp"


using namespace gazeestimation;

Vec3 calculatePointOfInterest(const Vec3& cornea_center, const Vec3& visual_axis_unit_vector, double z_shift)
{
	const double kg = (z_shift - cornea_center[2]) / visual_axis_unit_vector[2];
	return cornea_center + kg * visual_axis_unit_vector;
}

Vec2 estimateScreenPoint(const Vec3& poi, double screen_pixel_size_x, double screen_pixel_size_y)
{
	return make_vec2(poi[0] / screen_pixel_size_x, -poi[1] / screen_pixel_size_y);
}

/// Adapter from the return value of the calibration to the format that the optimization backend dictates
/// for the variables. This enables the use of the code that applies a set of variables to the input parameters
/// to apply the final result as well.
///
/// This relies on the (de-)allocation behavior of the vector class. No memory is allocated,
/// and the returned pointers are only valid as long as no operation that changes the layout of any of the original
/// vectors is made.
const double* const* const vecvecToPointerPointer(std::vector<std::vector<double>>& a, std::vector<double*>& tmp) {
	for (unsigned int i = 0; i < a.size(); i++)
	{
		tmp.push_back(&a[i][0]);
	}
	return &tmp[0];
}


Vec3 resultProcessor(const DefaultGazeEstimationResult& result, double z_shift, const Vec3& wcs_offset)
{
	return calculatePointOfInterest(result.center_of_cornea, result.visual_axis, z_shift) - wcs_offset;
}

/*
Vec3 resultProcessor(const DefaultGazeEstimationResult& result, double z_shift, const Vec3& wcs_offset)
{
	const double screen_size_cm_x = 48.7;
	const double screen_size_cm_y = 27.4;

	auto pos = calculatePointOfInterest(result.center_of_cornea, result.visual_axis, z_shift) - wcs_offset;
	if (pos[0] > 0 || pos[1] > 0 || pos[0] < screen_size_cm_x || pos[1] < screen_size_cm_y)
		return pos;
	return make_vec3(10000, 10000, 0);
}
*/




Vec2 cvToGeVec2(const cv::Point2f& a)
{
	return make_vec2(a.x, a.y);
}



/// Calibrates against alpha, beta, R, K, camera_rotation_y, camera_rotation_z
/// To change which variables are calibrated against also chagne calibrateGazeEstimationFrom,
/// which supplies the bounds and dictates the order of these.
EyeAndCameraParameters sixVariableCalibrationApplicator(EyeAndCameraParameters params, double const* const* variables)
{
	params.alpha = variables[0][0];
	params.beta = variables[1][0];
	params.R = variables[2][0];
	params.K = variables[3][0];
	params.cameras[0].set_camera_angle_y(variables[4][0]);
	params.cameras[0].set_camera_angle_z(variables[5][0]);

	//params.light_positions[0] = params.cameras[0].ccs_to_wcs(make_vec3(25.0, -1.0, -1));
	//params.light_positions[1] = params.cameras[0].ccs_to_wcs(make_vec3(0, -7.5, -4));
	//params.light_positions[2] = params.cameras[0].ccs_to_wcs(make_vec3(-25.0, -1.0, -1));

	params.light_positions[0] = params.cameras[0].ccs_to_wcs(make_vec3(25.1, 2.5, -1.0));
	params.light_positions[1] = params.cameras[0].ccs_to_wcs(make_vec3(0, -8.5, -2.0));
	params.light_positions[2] = params.cameras[0].ccs_to_wcs(make_vec3(-25.1, 2.5, -1.0));

	//params.light_positions[0] = params.cameras[0].ccs_to_wcs(make_vec3(25.0, 2.0, 1.5));
	//params.light_positions[1] = params.cameras[0].ccs_to_wcs(make_vec3(0, -8.5, -3.0));
	//params.light_positions[2] = params.cameras[0].ccs_to_wcs(make_vec3(-25.0, 2.0, 1.5));

	//params.light_positions[0] = params.cameras[0].ccs_to_wcs(make_vec3(25.0,  2.5,  -1.0));
	//params.light_positions[1] = params.cameras[0].ccs_to_wcs(make_vec3(0,     -7.5, -2.5));
	//params.light_positions[2] = params.cameras[0].ccs_to_wcs(make_vec3(-25.0, 2.5,   -1.0));

	//params.light_positions[0] = params.cameras[0].ccs_to_wcs(make_vec3(25.0, -1.0, -2.0));
	//params.light_positions[1] = params.cameras[0].ccs_to_wcs(make_vec3(0, -8.5, -2.0));
	//params.light_positions[2] = params.cameras[0].ccs_to_wcs(make_vec3(-25.0, -1.0, -2.0));


	return params;
}

