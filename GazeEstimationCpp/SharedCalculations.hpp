#ifndef SHARED_CALCULATIONS_HPP_INCLUDED
#define SHARED_CALCULATIONS_HPP_INCLUDED

#include "MathTypes.hpp"
#include "GazeEstimationTypes.hpp"

namespace gazeestimation
{
	inline Vec3 calculate_nu_ecs(double alpha, double beta)
	{
		return make_vec3(-sin(alpha) * cos(beta), sin(beta), cos(alpha) * cos(beta));
	}

	inline Vec3 calculate_eye_angles(const Vec3& optic_axis_unit_vector)
	{
		return make_vec3(
			-1 * atan(optic_axis_unit_vector[0] / optic_axis_unit_vector[2]),
			asin(optic_axis_unit_vector[1]),
			0
		);
	}

	/// Note that this is not equal to a general rotation matrix, as the coordinate system this 
	/// transforms from is flipped as well.
	inline Mat3x3 calculate_eye_rotation_matrix(double theta, double phi, double kappa)
	{
		Mat3x3 Rflip, Rtheta, Rphi, Rkappa;
		Rflip << -1, 0, 0,
			0, 1, 0,
			0, 0, -1;
		Rtheta << std::cos(theta), 0, -std::sin(theta),
			0, 1, 0,
			std::sin(theta), 0, std::cos(theta);
		Rphi << 1, 0, 0,
			0, std::cos(phi), std::sin(phi),
			0, -std::sin(phi), std::cos(phi);
		Rkappa << std::cos(kappa), -std::sin(kappa), 0,
			std::sin(kappa), std::cos(kappa), 0,
			0, 0, 1;

		return mat_prod(Rflip, mat_prod(Rtheta, mat_prod(Rphi, Rkappa)));

	}

	inline Vec3 calculate_visual_axis_unit_vector(const Vec3& optical_axis_unit_vector, double alpha, double beta)
	{
		const Vec3 nu_ecs = calculate_nu_ecs(alpha, beta);
		Vec3 eye_angles = calculate_eye_angles(optical_axis_unit_vector);
		const Mat3x3 Reye = calculate_eye_rotation_matrix(eye_angles[0], eye_angles[1], eye_angles[2]);
		return mat3vec3_prod(Reye, nu_ecs);
	}


	/// Calculates iota per eq 3.33
	inline Vec3 calculate_iota(const Vec3& camera_position, const Vec3& pupil_por_wcs, const Vec3& center_of_cornea, double R, double n1, double n2)
	{
		Vec3 zeta = normalized(camera_position - pupil_por_wcs);
		Vec3 eta = (pupil_por_wcs - center_of_cornea) / R;
		double eta_dot_zeta = dot(eta, zeta);

		double a = eta_dot_zeta - sqrt((n1 / n2)*(n1 / n2) - 1 + eta_dot_zeta * eta_dot_zeta);
		return (n2 / n1) * (a * eta - zeta);
	}

	/// Calculates kr per eq. 3.29
	inline double calculate_kr(const Vec3& camera_position, const Vec3& image_pupil_center, const Vec3& cornea_center, double R)
	{
		const double a = squared_length(camera_position - image_pupil_center);
		const double b = dot(camera_position - image_pupil_center, camera_position - cornea_center);
		const double c = squared_length(camera_position - cornea_center) - R * R;

		return (-b - sqrt(b * b - a * c)) / a;
	}

	inline Vec3 calculate_r(const Vec3& camera_position, const Vec3& pupil_image_wcs, const Vec3& cornea_wcs, double R)
	{
		const double kr = calculate_kr(camera_position, pupil_image_wcs, cornea_wcs, R);
		return camera_position + kr * (camera_position - pupil_image_wcs);
	}

}

#endif