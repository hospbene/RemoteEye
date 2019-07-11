#ifndef MATH_TYPES_HPP_INCLUDED
#define MATH_TYPES_HPP_INCLUDED

#include <Eigen/Core>

namespace gazeestimation {

	/// These types can be swapped out for anything that supports operators +,-,/,* with scalars
	/// and supplies a corresponding header file implementing the operations from Utils.cpp
	/// in addition they must define an operator <<= for matrics compliant with boost ubas's implementation
	typedef Eigen::Vector2d Vec2;
	typedef Eigen::Vector2i Vec2i;
	typedef Eigen::Vector3d Vec3;
	typedef Eigen::Matrix3d Mat3x3;

	inline Vec2 make_vec2(double a, double b)
	{
		return Vec2(a, b);
	}

	inline Vec3 make_vec3(double a, double b, double c)
	{
		return Vec3(a, b, c);
	}

	inline Mat3x3 mat_prod(const Mat3x3& a, const Mat3x3& b)
	{
		return a * b;
	}

	inline double dot(const Vec3& a, const Vec3& b)
	{
		return a.dot(b);
	}

	inline Vec3 mat3vec3_prod(const Mat3x3& a, const Vec3& b)
	{
		return a * b;
	}

	inline Vec3 normalized(const Vec3& a)
	{
		return a.normalized();
	}

	inline double length(const Vec3& a)
	{
		return a.norm();
	}

	inline double length(const Vec2& a)
	{
		return a.norm();
	}

	inline double squared_length(const Vec3& a)
	{
		return a.squaredNorm();
	}

	inline double squared_length_vec2(const Vec2& a)
	{
		return a.squaredNorm();
	}

	inline std::string vec3_to_string(const Vec3& a)
	{
		std::stringstream s;
		s << "(" << a[0] << ", " << a[1] << ", " << a[2] << ")";
		return s.str();
	}

	inline std::string vec2_to_string(const Vec3& a)
	{
		std::stringstream s;
		s << "(" << a[0] << ", " << a[1] << ")";
		return s.str();
	}

	inline Mat3x3 identity_matrix3x3()
	{
		return Eigen::Matrix3d::Identity();
	}

	inline Mat3x3 calculate_extrinsic_rotation_matrix(double alpha, double beta, double gamma)
	{
		Mat3x3 Rx, Ry, Rz;
		Rx << 1, 0, 0,
			0, std::cos(alpha), -std::sin(alpha),
			0, std::sin(alpha), std::cos(alpha);
		Ry << std::cos(beta), 0, std::sin(beta),
			0, 1, 0,
			-std::sin(beta), 0, std::cos(beta);
		Rz << std::cos(gamma), -std::sin(gamma), 0,
			std::sin(gamma), std::cos(gamma), 0,
			0, 0, 1;
		return mat_prod(Rz, mat_prod(Ry, Rx));
	}

	inline Vec3 cross_product(const Vec3& a, const Vec3& b)
	{
		return a.cross(b);
	}

	/// Returns the midpoint of the shortest segment between the two lines o1+a * d1 and o2 + b * d2;
	Vec3 shortest_line_segment(const Vec3& o1, const Vec3& d1, const Vec3& o2, const Vec3& d2);
}

#endif