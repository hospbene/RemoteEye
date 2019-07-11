// All equations are from 
// Remote, Non - Contact Gaze Estimation with Minimal Subject Cooperation
// Guestrin, Elias Daniel
// https://tspace.library.utoronto.ca/handle/1807/24349
#include "TwoCameraSpherical.hpp"

#include <ceres/ceres.h>

#include "PinholeCameraModel.hpp"
#include "Utils.hpp"
#include "SharedCalculations.hpp"

namespace gazeestimation {
	
	class ROptimizingCorneaDistance
	{
	private:
		const std::vector<Vec3>& glints;
		const std::vector<Vec3>& lights;
		const std::vector<Vec3>& camera_positions;
		double scale_r;

	public:
		/// \param	glints	List of glints so that glint i of camera j is at index j * glints_per_camera + i 
		/// \param	lights	List of the positions of the lights. The number and ordering of the lights must match the glints.
		/// \param	camera_positions	The positions of the cameras.
		/// \param	scale_r	A scaling factor for R. The true R used by this is variable_R / scale_R.
		ROptimizingCorneaDistance(const std::vector<Vec3>& glints,
									const std::vector<Vec3>& lights, 
									const std::vector<Vec3>& camera_positions,
									double scale_r)
			: glints(glints), lights(lights), camera_positions(camera_positions), scale_r(scale_r){}

		bool operator()(double const* const* variables, double* residual) const {
			double R =  *variables[0]/ scale_r;
			const unsigned int glints_per_camera = lights.size();

			// calculate the cornea centers that result from each of the glints under these variables
			// i and j swapped here to match the notation in the text
			std::vector<Vec3> cornea_centers;
			for(unsigned int j = 0; j < camera_positions.size(); j++)
			{
				const Vec3 camera_position = camera_positions[j];
				for(unsigned int i = 0; i < lights.size(); i++)
				{
					// k_i = *variables[1+i] 
					Vec3 q_ij = camera_position + *variables[1 + j * glints_per_camera + i] * normalized(camera_position - glints[j * glints_per_camera + i]);
					Vec3 c_ij = q_ij - R *  normalized(normalized(lights[i] - q_ij) + normalized(camera_position - q_ij));
					cornea_centers.push_back(c_ij);
				}
			}

			//TODO: only use unique pairs here.
			for(unsigned int i = 0; i < cornea_centers.size(); i++)
			{
				for(unsigned int j = 0; j < cornea_centers.size(); j++)
				{
					residual[i * cornea_centers.size() + j] = squared_length(cornea_centers[i] - cornea_centers[j]);
				}
			}
						
			return true;
		}
	};

	/// Calculates the cornea center using the methods detailed on p. 74f, employing eq. 3.23
	/// This does not need a previously calibrated R, or any specific setup, but does minimize numerically.
	Vec3 calculate_cornea_center_no_R(const PupilCenterGlintInputs& data, const EyeAndCameraParameters& parameters, double& r_out)
	{
		const double scale_R = 100;
		// reformulate some of the inputs in the interest of keeping the cost functor simpler
		// the glints are handed over as [camera1 glint1, camera 1 glint2, ..., camera 2 glint 1 ...]
		std::vector<Vec3> glints;
		std::vector<Vec3> camera_positions;
		for(unsigned int i = 0; i < parameters.cameras.size(); i++)
		{
			for (const auto& glint : data.data[i].glints)
			{
				glints.emplace_back(parameters.cameras[i].ics_to_wcs(glint));
			}
			camera_positions.push_back(parameters.cameras[i].position);
		}


		auto distance_corneas = new ROptimizingCorneaDistance(glints, parameters.light_positions, camera_positions, scale_R);

		ceres::Problem problem; 
		auto cost_function = new ceres::DynamicNumericDiffCostFunction<ROptimizingCorneaDistance, ceres::CENTRAL>(distance_corneas);


		cost_function->AddParameterBlock(1);
		for (unsigned int i = 0; i < glints.size(); i++)
		{
			cost_function->AddParameterBlock(1);
		}


		cost_function->SetNumResiduals(glints.size() * glints.size());

		double R = parameters.R*scale_R; // scale the R for the cost function
		std::unique_ptr<double> ks(new double[glints.size()]);
		
		std::vector<double*> variables;
		variables.push_back(&R);
		for (unsigned int i = 0; i < glints.size(); i++) {
			ks.get()[i] = parameters.distance_to_camera_estimate;
			variables.push_back(&ks.get()[i]);
		}

		problem.AddResidualBlock(cost_function, nullptr, variables);
		problem.SetParameterLowerBound(&R, 0, 0.3*scale_R);
		problem.SetParameterUpperBound(&R, 0, 2* scale_R);

		for (unsigned int i = 0; i < glints.size(); i++) {
			problem.SetParameterLowerBound(variables[i], 0, 2);
			problem.SetParameterUpperBound(variables[i], 0, 400);		
		}

		ceres::Solver::Options options;
		options.minimizer_progress_to_stdout = false;
		options.linear_solver_type = ceres::DENSE_QR;
		options.max_num_iterations = 1000;
		
		ceres::Solver::Summary summary;
		Solve(options, &problem, &summary);


		// calculate the cornea centers that result from each of the glints under these variables
		// i and j swapped here to match the notation in the text
		Vec3 cornea_center = make_vec3(0, 0, 0); 
		const unsigned int num_glints = parameters.light_positions.size();
		R = R / scale_R;
		for (unsigned int j = 0; j < camera_positions.size(); j++)
		{
			const Vec3 camera_position = camera_positions[j];
			for (unsigned int i = 0; i < parameters.light_positions.size(); i++)
			{
				// k_i = *variables[1+i] 
				Vec3 q_ij = camera_position + ks.get()[j * num_glints + i] * normalized(camera_position - glints[j * num_glints + i]);
				Vec3 c_ij = q_ij - R *  normalized(normalized(parameters.light_positions[i] - q_ij) + normalized(camera_position - q_ij));
				cornea_center += c_ij;
			}
		}

		cornea_center /= glints.size();
		
		r_out = R;

		return cornea_center;
	}

	/// Calculates optic axis per section 3.3.1 (without relying on any eye parameters, with explicit refraction model).
	/// only works as long as eye optic axis has no intersection with line between camera_position1 and camera_position2
	Vec3 calculate_optic_axis_unit_vector_explicit_refraction_i(const Vec3& camera1_pos, const Vec3& camera2_pos, 
						const Vec3& cornea_center, const Vec3& pupil1_image_wcs, const Vec3& pupil2_image_wcs)
	{
		
		Vec3 optic_axis = normalized(cross_product(
			cross_product(camera1_pos - pupil1_image_wcs, cornea_center - camera1_pos),
			cross_product(camera2_pos - pupil2_image_wcs, cornea_center - camera2_pos)));

		// there are two possible results here, make sure we choose the one that points outside of the eye in the correct direction. As our scene plane is at z=0
		if (cornea_center[2] > 0 && optic_axis[2] > 0)
			optic_axis = -optic_axis;

		return optic_axis;
	}

	const Vec3 calculate_optic_axis_unit_vector_explicit_refraction_ii(const PupilCenterGlintInputs& data,
		const EyeAndCameraParameters& parameters, const Vec3& cornea_center, double R)
	{
		std::vector<Vec3> iotas;
		std::vector<Vec3> rs;
		for (unsigned int i = 0; i < parameters.cameras.size(); i++)
		{
			const Vec3 pupil_wcs = parameters.cameras[i].ics_to_wcs(data.data[i].pupil_center);
			//std::cout << "-> "<< pupil_wcs << std::endl;
			iotas.push_back(normalized(calculate_iota(parameters.cameras[i].position, pupil_wcs, cornea_center, R, parameters.n1, parameters.n2)));
			rs.push_back(calculate_r(parameters.cameras[i].position, pupil_wcs, cornea_center, R));
		}

		Vec3 pupil_center = shortest_line_segment(rs[0], iotas[0], rs[1], iotas[1]);
		return normalized(pupil_center - cornea_center);
	}

	TwoCamSphericalGE::TwoCamSphericalGE(OpticAxisReconstructionMethod method): 
	optic_axis_method(method)
	{
		
	}

	DefaultGazeEstimationResult TwoCamSphericalGE::estimate(const PupilCenterGlintInputs& data, const EyeAndCameraParameters& parameters)
	{
		if (parameters.cameras.size() != 2)
		{
			throw std::exception("this method can only handle a single camera");
		}

		double estimated_R = parameters.R;
		const Vec3 cornea_center = calculate_cornea_center_no_R(data, parameters, estimated_R);

		const Vec3 pupil1_image_wcs = parameters.cameras[0].ics_to_wcs(data.data[0].pupil_center);
		const Vec3 pupil2_image_wcs = parameters.cameras[1].ics_to_wcs(data.data[1].pupil_center);

		Vec3 optic_axis_unit_vector = make_vec3(0, 0, 0);
		if(optic_axis_method == ExplicitRefraction1){
			optic_axis_unit_vector = calculate_optic_axis_unit_vector_explicit_refraction_i(
				parameters.cameras[0].position,
				parameters.cameras[1].position,
				cornea_center,
				pupil1_image_wcs,
				pupil2_image_wcs
			);
		}
		else if (optic_axis_method == ExplicitRefraction2)
		{
			optic_axis_unit_vector = calculate_optic_axis_unit_vector_explicit_refraction_ii(
				data, parameters, cornea_center, estimated_R
			);			
		}
		else
		{
			return DefaultGazeEstimationResult::make_error("configured optic axis reconstruction method does not exist.");
		}
		
		const Vec3 visual_axis_unit_vector = calculate_visual_axis_unit_vector(optic_axis_unit_vector, parameters.alpha, parameters.beta);
		
		DefaultGazeEstimationResult result;
		result.is_valid = true;
		result.center_of_cornea = cornea_center;
		result.optical_axis = optic_axis_unit_vector;
		result.visual_axis = visual_axis_unit_vector;
		return result;
	}

}
