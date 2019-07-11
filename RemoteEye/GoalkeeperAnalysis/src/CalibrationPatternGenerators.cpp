#include "CalibrationPatternGenerators.h"
#include <map>

FreeformPatternGenerator::FreeformPatternGenerator(unsigned int area_width, unsigned int area_height,
                                                   unsigned int num_points):
	area_width(area_width),
	area_height(area_height),
	num_points(num_points) { }


struct PatternDescription
{
	unsigned int horizontal_divider = 0;
	std::vector<double> horizontal_factors;
	unsigned int vertical_divider = 0;
	std::vector<double> vertical_factors;
	// the points that are hidden out of a regular grid.
	std::vector<unsigned int> hidden_points;
};

std::vector<std::pair<double, double>> FreeformPatternGenerator::calibrationPoints()
{
	//TODO: Check that this convoluted way of generating the points is actually neccessary, or a regular grid will do.
	PatternDescription description;
	switch (num_points)
	{
	case 5:
		description.horizontal_divider = 10;
		description.horizontal_factors = {1, 5, 9};
		description.vertical_divider = 15;
		description.vertical_factors = {2, 7.5, 12};
		description.hidden_points = {0, 2, 6, 8};
		break;
	case 9:
		description.horizontal_divider = 10;
		description.horizontal_factors = {2, 5, 8};
		description.vertical_divider = 15;
		description.vertical_factors = {3, 7, 11.5};
		//description.hidden_points = { 7 };
		break;
	case 16:
		description.horizontal_divider = 12;
		description.horizontal_factors = {1, 4, 7.5, 10.5};
		description.vertical_divider = 16;
		description.vertical_factors = {1, 5.5, 10, 14};
		break;
	case 25:
		description.horizontal_divider = 12;
		description.horizontal_factors = {1, 3.5, 6, 8.5, 11};
		description.vertical_divider = 16;
		description.vertical_factors = {1, 4, 8, 12, 15};
		break;
	default:
		throw std::exception("This number of points is not supported.");
	}

	std::vector<std::pair<double, double>> result;
	for (unsigned int y = 0; y < description.vertical_factors.size(); y++)
	{
		for (unsigned int x = 0; x < description.horizontal_factors.size(); x++)
		{
			int index = y * description.vertical_factors.size() + x;
			if (std::find(description.hidden_points.begin(), description.hidden_points.end(), index) != description
			                                                                                            .hidden_points.end())
				continue;

			result.emplace_back(std::make_pair(
				static_cast<double>(area_width) / description.horizontal_divider * description.horizontal_factors[x],
				static_cast<double>(area_height) / description.vertical_divider * description.vertical_factors[y]));
		}
	}
	return result;
}
