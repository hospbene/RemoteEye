// Gaze Estimation for one camera and two light sources
// All equations are from 
// Remote, Non - Contact Gaze Estimation with Minimal Subject Cooperation
// Guestrin, Elias Daniel
// https://tspace.library.utoronto.ca/handle/1807/24349
#ifndef TWO_CAMERA_SPHERICAL_HPP_INCLUDED
#define TWO_CAMERA_SPHERICAL_HPP_INCLUDED

#include "GazeEstimationTypes.hpp"

namespace gazeestimation {


	class TwoCamSphericalGE : public GazeEstimationMethod<EyeAndCameraParameters, PupilCenterGlintInputs, DefaultGazeEstimationResult>
	{
	public:
		enum OpticAxisReconstructionMethod
		{
			ExplicitRefraction1 = 0,
			ExplicitRefraction2
		};

		explicit TwoCamSphericalGE(OpticAxisReconstructionMethod method);
		DefaultGazeEstimationResult estimate(const PupilCenterGlintInputs& data, const EyeAndCameraParameters& parameters) override;

	private:
		OpticAxisReconstructionMethod optic_axis_method;
	};

}

#endif
