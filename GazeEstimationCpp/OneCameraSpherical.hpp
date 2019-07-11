// Gaze Estimation for one camera and two light sources
// All equations are from 
// Remote, Non - Contact Gaze Estimation with Minimal Subject Cooperation
// Guestrin, Elias Daniel
// https://tspace.library.utoronto.ca/handle/1807/24349
// Also contains techniques from
// A robust 3D eye gaze tracking system using noise reduction
// J Chen, Y Tong, W Gray, Q Ji - Proceedings of the 2008 symposium on Eye tracking …, 2008
#ifndef ONE_CAMERA_SPHERICAL_HPP_INCLUDED
#define ONE_CAMERA_SPHERICAL_HPP_INCLUDED

#include "GazeEstimationTypes.hpp"

namespace gazeestimation {


	class OneCamSphericalGE : public GazeEstimationMethod<EyeAndCameraParameters, PupilCenterGlintInputs, DefaultGazeEstimationResult>
	{
	public:
		/// A function that accords to this typedef is used as a hook for filtering. Given the current calculated value for the 
		/// specified quantity, it should return the value that should be used from here on.
		typedef std::function<Vec3(Vec3)> Vec3Filter;

		OneCamSphericalGE() = default;
		explicit OneCamSphericalGE(bool use_chen_noise_reduction);

		/// \brief Provides an extension point to filter the coordinate of the center of cornea in the world coordinate system.
		void setCorneaCenterFilter(Vec3Filter filter);
		/// \brief Provides an extension point to filter the coordinate of the virtual pupil center in the world coordinate system.
		void setPupilCenterFilter(Vec3Filter filter);

		DefaultGazeEstimationResult estimate(const PupilCenterGlintInputs& data, const EyeAndCameraParameters& parameters) override;
	private:
		bool use_chen_noise_reduction = false;

		Vec3Filter cornea_center_filter;
		Vec3Filter pupil_center_filter;
	};

}

#endif