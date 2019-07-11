// All equations are from 
// Remote, Non - Contact Gaze Estimation with Minimal Subject Cooperation
// Guestrin, Elias Daniel
// https://tspace.library.utoronto.ca/handle/1807/24349
// Also contains techniques from
// A robust 3D eye gaze tracking system using noise reduction
// J Chen, Y Tong, W Gray, Q Ji - Proceedings of the 2008 symposium on Eye tracking …, 2008
#include "OneCameraSpherical.hpp"

#include <utility>

#include <ceres/ceres.h>

#include "PinholeCameraModel.hpp"
#include "Utils.hpp"
#include "SharedCalculations.hpp"


namespace gazeestimation {


	Vec3 calculate_q(double kq, const Vec3& o, const Vec3& u)
	{
		return o + kq * normalized(o - u);
	}

	Vec3 calculate_cornea_center(const Vec3& q, const Vec3& light, const Vec3& camera_position, const double R)
	{
		const Vec3 l_q_unit = normalized(light - q);
		const Vec3 o_q_unit = normalized(camera_position - q);
		return q - R * normalized(l_q_unit + o_q_unit);
	}

	class DistanceBetweenCorneasFunctor
	{
	private:
		const std::vector<Vec3>* const glints;
		const std::vector<Vec3>* const lights;
		double R;
		const Vec3 camera_position;

	public:
		DistanceBetweenCorneasFunctor(const std::vector<Vec3>* const glints, const std::vector<Vec3>* const lights, double r,
			Vec3 camera_position)
			: glints(glints),
			lights(lights),
			R(r),
			camera_position(std::move(camera_position)) {}

		bool operator()(double const* const* variables, double* residual) const {
			std::vector<Vec3> cs;

			for (unsigned int i = 0; i < glints->size(); i++)
			{
				double kq = variables[i][0];
				Vec3 q = calculate_q(kq, camera_position, (*glints)[i]);
				Vec3 c = calculate_cornea_center(q, (*lights)[i], camera_position, R);
				cs.push_back(c);
			}

			size_t index = 0;
			for(int i = 0; i < glints->size(); i++)
			{
				for(int j = 0; j < i; j++)
				{
					Vec3 d = cs[i] - cs[j];
					residual[index++] = d[0];
					residual[index++] = d[1];
					residual[index++] = d[2];
				}
			}
			
			//std::cout << *x << "," << *y << " | " << residual[0] <<   std::endl;;
			return true;
		}
	};

	Vec3 calculate_cornea_center_wcs(const std::vector<Vec3>* const glints,
		const std::vector<Vec3>* const lights,
		const Vec3& camera_position, double R, double camera_eye_distance_estimate)
	{
		auto distanceCorneas = new DistanceBetweenCorneasFunctor(glints, lights, R, camera_position);

		ceres::Problem problem;
		//ceres::CostFunction* cost_function = new ceres::NumericDiffCostFunction<DistanceBetweenCorneasFunctor, ceres::CENTRAL, 3, 1, 1>(distanceCorneas);
		auto cost_function = new ceres::DynamicNumericDiffCostFunction<DistanceBetweenCorneasFunctor, ceres::CENTRAL>(distanceCorneas);
		
		for (unsigned int i = 0; i < glints->size(); i++)
		{
			cost_function->AddParameterBlock(1);
		}

		cost_function->SetNumResiduals(3 * 0.5 * (glints->size() * glints->size() - glints->size()));

		std::unique_ptr<double> ks(new double[glints->size()]);

		std::vector<double*> variables;
		for (unsigned int i = 0; i < glints->size(); i++) {
			ks.get()[i] = camera_eye_distance_estimate;
			variables.push_back(&ks.get()[i]);
		}

		problem.AddResidualBlock(cost_function, nullptr, variables);

		for (unsigned int i = 0; i < glints->size(); i++) {
			problem.SetParameterLowerBound(variables[i], 0, 2);
			problem.SetParameterUpperBound(variables[i], 0, 400);
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
		/*	std::cout << summary.FullReport() << std::endl;
		std::cout << std::endl;
		std::cout << summary.IsSolutionUsable() << std::endl;
		std::cout << std::endl;*/

		/*
		for (int i = 0; i < glints->size(); i++)
		std::cout << variables[i] << ", ";
		std::cout << std::endl;
		*/
		//TODO: Solution usability should really be checked here.

		Vec3 c_total = make_vec3(0, 0, 0);
		for (unsigned int i = 0; i < glints->size(); i++)
		{
			Vec3 q = calculate_q(variables[i][0], camera_position, (*glints)[i]);
			Vec3 c = calculate_cornea_center(q, (*lights)[i], camera_position, R);
			c_total += c;
		}

		return c_total / static_cast<double>(glints->size());
	}

	Vec3 calculate_cornea_center(std::vector<Vec2> glints, const EyeAndCameraParameters& parameters)
	{
		std::vector<Vec3> glints_wcs;
		/*for (const auto& glint : glints)
		{
			glints_wcs.push_back(parameters.cameras[0].ics_to_wcs(glint));
		}*/

		std::vector<Vec3> selected_lights;

		for(int i = 0; i < glints.size(); i++)
		{
			if (!glintValid(glints[i]))
				continue;
			glints_wcs.push_back(parameters.cameras[0].ics_to_wcs(glints[i]));
			selected_lights.push_back(parameters.light_positions[i]);
		}

		return calculate_cornea_center_wcs(&glints_wcs,
			&selected_lights,
			parameters.cameras[0].position,
			parameters.R, parameters.distance_to_camera_estimate);
	}


	Vec3 calculate_p(const Vec3& camera_position, const Vec3& pupil_por_wcs, const Vec3& center_of_cornea, double R, double K, double n1, double n2)
	{
		Vec3 iota = calculate_iota(camera_position, pupil_por_wcs, center_of_cornea, R, n1, n2);
		double rc_dot_iota = dot((pupil_por_wcs - center_of_cornea), iota);
		double kp = -1 * rc_dot_iota - sqrt(rc_dot_iota*rc_dot_iota - (R * R - K * K));
		return pupil_por_wcs + kp * iota;
	}

	Vec3 calculate_optic_axis_unit_vector(const Vec3& pupil_wcs, const Vec3& camera_position, const Vec3& center_of_cornea, 
		double R, double K, double n1, double n2, bool use_chen_noise_reduction)
	{
		const Vec3 pupil_por_wcs = calculate_r(camera_position, pupil_wcs, center_of_cornea, R);

		Vec3 pupil_center_wcs = calculate_p(camera_position, pupil_por_wcs, center_of_cornea, R, K, n1, n2);

		if(use_chen_noise_reduction)
		{
			double cxpx = center_of_cornea[0] - pupil_center_wcs[0];
			double cypy = center_of_cornea[1] - pupil_center_wcs[1];
			pupil_center_wcs[2] = center_of_cornea[2] - sqrt(K*K - cxpx * cxpx - cypy*cypy);
		}

		return normalized(pupil_center_wcs - center_of_cornea);
	}


	OneCamSphericalGE::OneCamSphericalGE(bool use_chen_noise_reduction):
	use_chen_noise_reduction(use_chen_noise_reduction)
	{
		
	}

	void OneCamSphericalGE::setCorneaCenterFilter(Vec3Filter filter)
	{
		cornea_center_filter = filter;
	}

	void OneCamSphericalGE::setPupilCenterFilter(Vec3Filter filter)
	{
		pupil_center_filter = filter;
	}

	DefaultGazeEstimationResult OneCamSphericalGE::estimate(const PupilCenterGlintInputs& data, const EyeAndCameraParameters& parameters)
	{
		if (data.data.size() != 1 || data.data[0].glints.size() < 2)
		{
			throw std::exception("this method must have one pair of pupil center/glint info with info about at least two glints.");
		}

		if (parameters.cameras.size() != 1)
		{
			throw std::exception("this method can only handle a single camera");
		}

		int valid_glints = 0;
		for(const auto& glint : data.data[0].glints)
		{
			if (glintValid(glint))
				valid_glints++;
		}

		if (valid_glints < 2)
			throw std::exception("There need to be at least 2 valid glints present.");

		const PinholeCameraModel camera = parameters.cameras[0];

		Vec3 cornea_center = calculate_cornea_center(data.data[0].glints, parameters);
		
		if(cornea_center_filter)
		{
			cornea_center = cornea_center_filter(cornea_center);
		}


		Vec3 pupil_wcs = camera.ics_to_wcs(data.data[0].pupil_center);

		if(pupil_center_filter)
		{
			pupil_wcs = pupil_center_filter(pupil_wcs);
		}

		const Vec3 optic_axis_unit_vector = calculate_optic_axis_unit_vector(pupil_wcs, parameters.cameras[0].position, cornea_center,
			parameters.R, parameters.K, parameters.n1, parameters.n2, use_chen_noise_reduction);

		const Vec3 visual_axis_unit_vector = calculate_visual_axis_unit_vector(optic_axis_unit_vector, parameters.alpha, parameters.beta);

		DefaultGazeEstimationResult result;
		result.is_valid = true;
		result.center_of_cornea = cornea_center;
		result.optical_axis = optic_axis_unit_vector;
		result.visual_axis = visual_axis_unit_vector;
		return result;
	}

}
