#ifndef UTILS_HPP_INCLUDED
#define UTILS_HPP_INCLUDED

#include "GazeEstimationTypes.hpp"

namespace gazeestimation {
	inline bool glintValid(const Vec2& glint) 
	{
		return glint[0] >= 0 && glint[1] >= 0;
	}
}

#endif