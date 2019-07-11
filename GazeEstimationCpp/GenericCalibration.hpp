#ifndef GENERIC_CALIBRATION_HPP_INCLUDED
#define GENERIC_CALIBRATION_HPP_INCLUDED

#include "GazeEstimationTypes.hpp"
#include <functional>
#include "Utils.hpp"
#include <ceres/ceres.h>

namespace gazeestimation {
	/// Takes a list of parameters, sample data together with truth, and minimizes the squared difference
	template <class Parameters, class InputData, class GazeEstimationResult>
	class GenericCalibration
	{
	public:
		typedef std::vector<std::pair<InputData, Vec3>> CalibrationDataMap;
		typedef std::function<Parameters(Parameters, double const* const*)> ParameterApplicator;
		typedef std::function<Vec3(const GazeEstimationResult&)> ResultProcessor;
	private:

	public:
		std::vector<std::vector<double>> calibrate(GazeEstimationMethod<Parameters, InputData, GazeEstimationResult>& estimation,
			Parameters& parameters,
			ParameterApplicator applicator,
			ResultProcessor result_processor,
			CalibrationDataMap& data,
			std::vector<std::vector<double>> initial_values,
			std::vector<std::vector<std::pair<double, double>>> bounds);

	};

	template <class Parameters, class InputData, class GazeEstimationResult>
	class CalibrationErrorFunctor
	{
	private:
		GazeEstimationMethod<Parameters, InputData, GazeEstimationResult>* const gaze_estimation;
		const typename GenericCalibration<Parameters, InputData, GazeEstimationResult>::CalibrationDataMap* const data;
		typename GenericCalibration<Parameters, InputData, GazeEstimationResult>::ParameterApplicator applicator;
		typename GenericCalibration<Parameters, InputData, GazeEstimationResult>::ResultProcessor result_processor;
		const Parameters parameters;

	public:

		CalibrationErrorFunctor(GazeEstimationMethod<Parameters, InputData, GazeEstimationResult>* const gaze_estimation,
			const typename GenericCalibration<Parameters, InputData, GazeEstimationResult>::CalibrationDataMap* const data,
			typename GenericCalibration<Parameters, InputData, GazeEstimationResult>::ParameterApplicator applicator,
			typename GenericCalibration<Parameters, InputData, GazeEstimationResult>::ResultProcessor result_proccessor,
			const Parameters parameters
		) :
			gaze_estimation(gaze_estimation),
			data(data),
			applicator(applicator),
			result_processor(result_proccessor),
			parameters(parameters)
		{

		}

		bool operator()(double const* const* variables, double* residual) const {

			Parameters our_parameters = applicator(parameters, variables);

			double error = 0;
			Vec3 error_vec = make_vec3(0,0,0);
			int index = 0;
			for (auto It = data->begin(); It != data->end(); ++It)
			{
				InputData data_in = (*It).first;
				Vec3 truth = (*It).second;

				GazeEstimationResult result = gaze_estimation->estimate(data_in, our_parameters);
				const Vec3 estimate = result_processor(result);
				const Vec3 diff = truth - estimate;
								
				residual[index++] = diff[0];
				residual[index++] = diff[1];
				residual[index++] = diff[2];
			}


			return true;
		}
	};



	template <class Parameters, class InputData, class GazeEstimationResult>
	std::vector<std::vector<double>> GenericCalibration<Parameters, InputData, GazeEstimationResult>::calibrate(
		GazeEstimationMethod<Parameters, InputData, GazeEstimationResult>& estimation,
		Parameters& parameters,
		ParameterApplicator applicator,
		ResultProcessor result_processor,
		CalibrationDataMap& data,
		std::vector<std::vector<double>> initial_values,
		std::vector<std::vector<std::pair<double, double>>> bounds)
	{
		assert(bounds.size() == initial_values.size());

		ceres::Problem problem;
		auto cost_function = new ceres::DynamicNumericDiffCostFunction<CalibrationErrorFunctor<Parameters, InputData, GazeEstimationResult>, ceres::CENTRAL>
			(new CalibrationErrorFunctor<Parameters, InputData, GazeEstimationResult>(&estimation, &data, applicator, result_processor, parameters));
		for (int i = 0; i < initial_values.size(); i++)
		{
			cost_function->AddParameterBlock(initial_values[i].size());
		}
		cost_function->SetNumResiduals(data.size() * 3);

		std::vector<double*> variables;
		for(unsigned int i = 0; i < initial_values.size(); i++)
		{
			double* element = new double[initial_values[i].size()];
			for(int j = 0; j < initial_values[i].size(); j++)
			{
				element[j] = initial_values[i][j];
			}
			variables.push_back(element);
		}
		problem.AddResidualBlock(cost_function, nullptr, variables);

		for(unsigned int i = 0; i < initial_values.size(); i++)
		{
			for (unsigned int j = 0; j < bounds[i].size(); j++) {
				double* element = variables[i];
				problem.SetParameterLowerBound(element, j, bounds[i][j].first);
				problem.SetParameterUpperBound(element, j, bounds[i][j].second);
			}
		}
		
		ceres::Solver::Options options;
		options.minimizer_progress_to_stdout = false;
		options.linear_solver_type = ceres::DENSE_QR;
		//	options.function_tolerance = 1e-8;
		//	options.gradient_tolerance = 1e-12;
		options.max_num_iterations = 1e4;
		//	options.min_line_search_step_size = 1e-3;
		//	options.use_nonmonotonic_steps = true;

		ceres::Solver::Summary summary;
		Solve(options, &problem, &summary);
		std::cout << summary.FullReport() << std::endl;
		std::cout << std::endl;
		std::cout << summary.IsSolutionUsable() << std::endl;
		std::cout << std::endl;

		std::vector<std::vector<double>> result;
		for(unsigned int i = 0; i < variables.size(); i++)
		{
			std::vector<double> this_variable;
			for(unsigned int j = 0; j < initial_values[i].size(); j++)
			{
				this_variable.push_back(variables[i][j]);
			}
			result.push_back(this_variable);
		}
		//TODO: Memory deallocation
		return result;
	}

}

#endif