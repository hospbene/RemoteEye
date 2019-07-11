#ifndef INPUT_OUTPUT_HELPERS_INCLUDED
#define INPUT_OUTPUT_HELPERS_INCLUDED

#include <fstream>

#include <boost/tokenizer.hpp>

#include "GazeEstimationTypes.hpp"
#include "GenericCalibration.hpp"

inline std::string read_file(const std::wstring& filename)
{
	std::ifstream input;
	input.open(filename);

	if (input.fail())
	{
		std::wcout << "Couldn't read from " << filename << std::endl;
		return "";
	}

	std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

	input.close();

	return content;
}

/// Reads the input csv, and returns (input data, truth)
inline std::vector<std::pair<gazeestimation::PupilCenterGlintInputs, gazeestimation::Vec2>> read_input_file(const std::wstring& filename)
{
	std::stringstream content = std::stringstream(read_file(filename));

	std::vector<std::pair<gazeestimation::PupilCenterGlintInputs, gazeestimation::Vec2>> res;
	std::string line;
	while (std::getline(content, line, '\n'))
	{
		boost::tokenizer<boost::escaped_list_separator<char>> csv_tokenizer(
			line, boost::escaped_list_separator<char>('\\', ',', '\"'));
		std::vector<std::string> columns;
		std::copy(csv_tokenizer.begin(), csv_tokenizer.end(), std::back_inserter(columns));
		assert(columns.size() >= 10);

		gazeestimation::PupilCenterGlintInputs pcgis;
		gazeestimation::PupilCenterGlintInput pcgi;
		pcgi.pupil_center = gazeestimation::make_vec2(std::stod(columns[2]), std::stod(columns[3]));
		pcgi.glints.push_back(gazeestimation::make_vec2(std::stod(columns[4]), std::stod(columns[5])));
		pcgi.glints.push_back(gazeestimation::make_vec2(std::stod(columns[8]), std::stod(columns[9])));
		pcgis.data.push_back(pcgi);
		res.push_back(std::make_pair(pcgis, gazeestimation::make_vec2(std::stod(columns[0]), std::stod(columns[1]))));
	}
	return res;
}

/// temporary method to read in debug data for which the intermediate results are known before this is integrated into a pipeline where it gets actual data.
inline std::vector<std::pair<gazeestimation::PupilCenterGlintInputs, gazeestimation::Vec2>> read_input_file_twocameras(const std::wstring& filename)
{
	std::stringstream content = std::stringstream(read_file(filename));

	std::vector<std::pair<gazeestimation::PupilCenterGlintInputs, gazeestimation::Vec2>> res;
	std::string line;
	while (std::getline(content, line, '\n'))
	{
		boost::tokenizer<boost::escaped_list_separator<char>> csv_tokenizer(
			line, boost::escaped_list_separator<char>('\\', ',', '\"'));
		std::vector<std::string> columns;
		std::copy(csv_tokenizer.begin(), csv_tokenizer.end(), std::back_inserter(columns));

		int num_cameras = std::stoi(columns[3]);
		int num_lights = std::stoi(columns[4]);

		int start_of_vars = 5;
		int start_of_glints = start_of_vars + num_cameras * 2;
		gazeestimation::PupilCenterGlintInputs pcgis;
		for (int i = 0; i < num_cameras; i++)
		{
			int pupil_index = start_of_vars + i * 2;
			gazeestimation::PupilCenterGlintInput pcgi;
			pcgi.pupil_center = gazeestimation::make_vec2(std::stod(columns[pupil_index]), std::stod(columns[pupil_index+1]));
			for(int j = 0; j < num_lights; j++)
			{
				int glint_index = start_of_glints + num_lights * 2 * i +  j * 2;
				pcgi.glints.push_back(gazeestimation::make_vec2(std::stod(columns[glint_index]), std::stod(columns[glint_index + 1])));
			}
			pcgis.data.push_back(pcgi);			
		}
		res.push_back(std::make_pair(pcgis, gazeestimation::make_vec2(std::stod(columns[0]), std::stod(columns[1]))));
	}
	return res;
}


inline double deg_to_rad(double a)
{
	return a * 3.141592653589793 / 180.;
}

inline double rad_to_deg(double a)
{
	return (a * 180.) / 3.141592653589793;
}

inline std::string gaze_estimation_result_to_string(const gazeestimation::DefaultGazeEstimationResult& r)
{
	std::stringstream ss("");
	ss << "Valid: " << r.is_valid << "\n";
	ss << "Center of Cornea\t" << gazeestimation::vec3_to_string(r.center_of_cornea) << "\n";
	ss << "Optical Axis\t" << gazeestimation::vec3_to_string(r.optical_axis) << "\n";
	ss << "Visual Axis\t" << gazeestimation::vec3_to_string(r.visual_axis) << "\n";
	return ss.str();
}

#endif