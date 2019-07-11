#ifndef PINHOLE_CAMERA_MODEL_HPP_INCLUDED
#define PINHOLE_CAMERA_MODEL_HPP_INCLUDED

#include "MathTypes.hpp"


namespace gazeestimation{
class PinholeCameraModel
{
private:
	Vec3 camera_angles;
	Mat3x3 actual_rotation_matrix;

public:
	// camera intrinsic
	double principal_point_x;
	double principal_point_y;
	double pixel_size_cm_x;
	double pixel_size_cm_y;
	double effective_focal_length_cm;

	/// the position in WCS
	Vec3 position;

	PinholeCameraModel():
		camera_angles(make_vec3(0,0,0)),
		actual_rotation_matrix(identity_matrix3x3()), 
		principal_point_x(0), 
		principal_point_y(0),
		pixel_size_cm_x(0),
		pixel_size_cm_y(0),
		effective_focal_length_cm(0) { }

	void set_camera_angles(double x, double y, double z)
	{
		camera_angles = make_vec3(x, y, z);
		actual_rotation_matrix = calculate_extrinsic_rotation_matrix(camera_angles[0], camera_angles[1], camera_angles[2]);
	}


	/// Returns the rotation matrix for this camera.
	Mat3x3 rotation_matrix() const {
		return actual_rotation_matrix;
	}

	/// Transforms the given vector in this camera's image coordinate system to 
	/// the camera coordinate system
	Vec3 ics_to_ccs(const Vec2& pos) const
	{
		return make_vec3(
			(pos[0] - principal_point_x) * pixel_size_cm_x,
			(pos[1] - principal_point_y) * pixel_size_cm_y,
			-effective_focal_length_cm
		);
	}

	Vec3 ccs_to_wcs(const Vec3& pos) const
	{
		return rotation_matrix() * pos + position;
	}

	Vec3 ics_to_wcs(const Vec2& pos) const
	{
		return ccs_to_wcs(ics_to_ccs(pos));
	}

	double camera_angle_x() const
	{
		return camera_angles[0];
	}

	double camera_angle_y() const
	{
		return camera_angles[1];
	}
	double camera_angle_z() const
	{
		return camera_angles[2];
	}

	void set_camera_angle_x(double angle)
	{
		set_camera_angles(angle, camera_angles[1], camera_angles[2]);
	}

	void set_camera_angle_y(double angle)
	{
		set_camera_angles(camera_angles[0], angle, camera_angles[2]);
	}

	void set_camera_angle_z(double angle)
	{
		set_camera_angles(camera_angles[0], camera_angles[1], angle);
	}

};

}

#endif