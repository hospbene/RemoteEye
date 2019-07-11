#include "MathTypes.hpp"

#include <Eigen/Dense>

namespace gazeestimation
{
	Vec3 shortest_line_segment(const Vec3& o1, const Vec3& d1, const Vec3& o2, const Vec3& d2)
	{
		Eigen::Matrix3Xd mat1(3,2);
		mat1 << d1[0], d2[0],
			d1[1], d2[1],
			d1[2], d2[2];
		Eigen::Matrix2d mat2_original;
		mat2_original << (dot(d1, d1)), -dot(d1, d2),
			-dot(d1, d2), dot(d2, d2);
		auto mat2_inverse = mat2_original.inverse();
		Eigen::Matrix2Xd mat3(2, 1);
		mat3 << -dot(d1, o1 - o2),
			dot(d2, o1 - o2);
		return 0.5 * mat1 * mat2_inverse * mat3 + 0.5 * (o1 + o2);
	}
}
