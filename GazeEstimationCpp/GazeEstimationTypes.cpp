#include "GazeEstimationTypes.hpp"

namespace gazeestimation
{
	DefaultGazeEstimationResult::DefaultGazeEstimationResult():
		is_valid(false),
		is_error(false),
		center_of_cornea(0, 0, 0),
		visual_axis(0, 0, 0),
		optical_axis(0, 0, 0),
		error("") { }

	DefaultGazeEstimationResult DefaultGazeEstimationResult::make_error(std::string error)
	{
		DefaultGazeEstimationResult res;
		res.is_valid = false;
		res.is_error = true;
		res.error = error;
		return res;
	}
}
