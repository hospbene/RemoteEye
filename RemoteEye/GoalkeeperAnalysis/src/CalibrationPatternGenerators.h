#ifndef CALIBRATION_PATTERN_GENERATORS_H_
#define CALIBRATION_PATTERN_GENERATORS_H_

#include <vector>

class CalibrationPatternGenerator
{
public:
	virtual ~CalibrationPatternGenerator() = default;

	virtual std::vector<std::pair<double, double>> calibrationPoints() = 0;
};

/// Generates calibration points in a grid in the fashion that the original implementation does
/// Consequently, this decides on the layout automatically, but only works for the fixed sets.
//TODO: Check if this is actually neccessary.
class FreeformPatternGenerator : public CalibrationPatternGenerator
{
public:
	explicit FreeformPatternGenerator(unsigned int area_width, unsigned int area_height, unsigned int num_points);

	std::vector<std::pair<double, double>> calibrationPoints() override;

private:
	unsigned int area_width;
	unsigned int area_height;
	unsigned int num_points;
};

#endif
