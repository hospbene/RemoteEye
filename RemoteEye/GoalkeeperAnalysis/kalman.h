#ifndef KALMAN_H
#define KALMAN_H

#include <opencv2/opencv.hpp>

#define GLINTFILTER_DYNAMIC_PARAMETERS  int(4) // [x,y,dx,dy]
#define GLINTFILTER_MEASURED_PARAMETERS int(2) // [x,y]
#define GLINTFILTER_CONTROL_PARAMETERS  int(0) // []

#define GLINTFILTER_DEFAULT_VARX float(1.5000f)
#define GLINTFILTER_DEFAULT_VARY float(1.5000f)
#define GLINTFILTER_DEFAULT_VARP float(1.0001f)

namespace KALMAN {

	class GlintFilter {
	private:

		/**
		* @brief cv kalman filter instance
		*/
		cv::KalmanFilter m_kf;

		/**
		* @brief indicates if kalman filter is initialized and ready to use
		*/
		bool m_initialized;

		/**
		* @brief expected error in x direction
		*/
		float m_varx;

		/**
		* @brief expected error in y direction
		*/
		float m_vary;

		/**
		* @brief TODO
		*/
		float m_varp;

		// KF states
		float m_transition_matrix[GLINTFILTER_DYNAMIC_PARAMETERS*GLINTFILTER_DYNAMIC_PARAMETERS*500];
		float m_measurement_matrix[GLINTFILTER_MEASURED_PARAMETERS*GLINTFILTER_DYNAMIC_PARAMETERS * 500];
		float m_processs_noise_matrix[GLINTFILTER_DYNAMIC_PARAMETERS*GLINTFILTER_DYNAMIC_PARAMETERS * 500];
		float m_measurement_noise_matrix[GLINTFILTER_MEASURED_PARAMETERS*GLINTFILTER_MEASURED_PARAMETERS * 500];
		float m_post_error_matrix[GLINTFILTER_MEASURED_PARAMETERS*GLINTFILTER_MEASURED_PARAMETERS * 500];
		float m_state_pre_matrix[GLINTFILTER_DYNAMIC_PARAMETERS * 500];
		float m_measurement[GLINTFILTER_MEASURED_PARAMETERS ];
	protected:

		void initialize(const float& x, const float& y) {
			// create KF instance
			this->m_kf = cv::KalmanFilter(
				GLINTFILTER_DYNAMIC_PARAMETERS,
				GLINTFILTER_MEASURED_PARAMETERS,
				GLINTFILTER_CONTROL_PARAMETERS);

			// initialize transition matrix
			// [ 1 0 1 0
			//   0 1 0 1
			//   0 0 1 0
			//   0 0 0 1 ]
			this->m_transition_matrix[0] = 1.0f;
			this->m_transition_matrix[1] = 0.0f;
			this->m_transition_matrix[2] = 1.0f;
			this->m_transition_matrix[3] = 0.0f;
			this->m_transition_matrix[4] = 0.0f;
			this->m_transition_matrix[5] = 1.0f;
			this->m_transition_matrix[6] = 0.0f;
			this->m_transition_matrix[7] = 1.0f;
			this->m_transition_matrix[8] = 0.0f;
			this->m_transition_matrix[9] = 0.0f;
			this->m_transition_matrix[10] = 1.0f;
			this->m_transition_matrix[11] = 0.0f;
			this->m_transition_matrix[12] = 0.0f;
			this->m_transition_matrix[13] = 0.0f;
			this->m_transition_matrix[14] = 0.0f;
			this->m_transition_matrix[15] = 1.0f;
			this->m_kf.transitionMatrix = cv::Mat1f(4, 4, this->m_transition_matrix);

			// initialize measurement matrix
			// [ 1 0 0 0
			//   0 1 0 0 ]
			this->m_measurement_matrix[0] = 1.0f;
			this->m_measurement_matrix[1] = 0.0f;
			this->m_measurement_matrix[2] = 0.0f;
			this->m_measurement_matrix[3] = 0.0f;
			this->m_measurement_matrix[4] = 0.0f;
			this->m_measurement_matrix[5] = 1.0f;
			this->m_measurement_matrix[6] = 0.0f;
			this->m_measurement_matrix[7] = 0.0f;
			this->m_kf.measurementMatrix = cv::Mat1f(2, 4, this->m_measurement_matrix);

			// initialize process noise matrix
			// [ 1 0 0 0
			//   0 1 0 0
			//   0 0 1 0
			//   0 0 0 1 ]
			this->m_processs_noise_matrix[0] = this->m_varp;
			this->m_processs_noise_matrix[1] = 0.0f;
			this->m_processs_noise_matrix[2] = 0.0f;
			this->m_processs_noise_matrix[3] = 0.0f;
			this->m_processs_noise_matrix[4] = 0.0f;
			this->m_processs_noise_matrix[5] = this->m_varp;
			this->m_processs_noise_matrix[6] = 0.0f;
			this->m_processs_noise_matrix[7] = 0.0f;
			this->m_processs_noise_matrix[8] = 0.0f;
			this->m_processs_noise_matrix[9] = 0.0f;
			this->m_processs_noise_matrix[10] = this->m_varp;
			this->m_processs_noise_matrix[11] = 0.0f;
			this->m_processs_noise_matrix[12] = 0.0f;
			this->m_processs_noise_matrix[13] = 0.0f;
			this->m_processs_noise_matrix[14] = 0.0f;
			this->m_processs_noise_matrix[15] = this->m_varp;
			this->m_kf.processNoiseCov = cv::Mat1f(4, 4, this->m_processs_noise_matrix);

			// initialize measurement noise matrix
			// [ 1 0
			//   0 1 ]
			this->m_measurement_noise_matrix[0] = this->m_varx;
			this->m_measurement_noise_matrix[1] = 0.0f;
			this->m_measurement_noise_matrix[2] = 0.0f;
			this->m_measurement_noise_matrix[3] = this->m_vary;
			this->m_kf.measurementNoiseCov = cv::Mat1f(2, 2, this->m_measurement_noise_matrix);

			// initialize post error matrix
			// [ 1 0 0 0
			//   0 1 0 0
			//   0 0 1 0
			//   0 0 0 1 ]
			this->m_post_error_matrix[0] = 0.1f;
			this->m_post_error_matrix[1] = 0.0f;
			this->m_post_error_matrix[2] = 0.0f;
			this->m_post_error_matrix[3] = 0.0f;
			this->m_post_error_matrix[4] = 0.0f;
			this->m_post_error_matrix[5] = 0.1f;
			this->m_post_error_matrix[6] = 0.0f;
			this->m_post_error_matrix[7] = 0.0f;
			this->m_post_error_matrix[8] = 0.0f;
			this->m_post_error_matrix[9] = 0.0f;
			this->m_post_error_matrix[10] = 0.1f;
			this->m_post_error_matrix[11] = 0.0f;
			this->m_post_error_matrix[12] = 0.0f;
			this->m_post_error_matrix[13] = 0.0f;
			this->m_post_error_matrix[14] = 0.0f;
			this->m_post_error_matrix[15] = 0.1f;
			this->m_kf.errorCovPost = cv::Mat1f(4, 4, this->m_post_error_matrix);

			// initialize pre states
			// [ x
			//   y
			//   0
			//   0 ]
			this->m_state_pre_matrix[0] = x;
			this->m_state_pre_matrix[1] = y;
			this->m_state_pre_matrix[2] = 0.0f;
			this->m_state_pre_matrix[3] = 0.0f;
			this->m_kf.statePre = cv::Mat1f(4, 1, this->m_state_pre_matrix);

			// initialize measurement
			// [ 0
			//   0 ]
			this->m_measurement[0] = 0.0f;
			this->m_measurement[1] = 0.0f;

			// all done!
			this->m_initialized = true;
		}

	public:

		/**
		* @param varx expected measurement noise in x direction
		* @param vary expected measurement noise in y direction
		* @param varp expected process error
		*/
		GlintFilter(float varx, float vary, float varp) :
			m_initialized(false),
			m_varx(varx),
			m_vary(vary),
			m_varp(varp) {

		}

		/**
		* @param varx expected measurement noise in x direction
		* @param vary expected measurement noise in y direction
		*/
		GlintFilter(float varx, float vary) :
			m_initialized(false),
			m_varx(varx),
			m_vary(vary),
			m_varp(GLINTFILTER_DEFAULT_VARP) {

		}

		/**
		*/
		GlintFilter() : m_initialized(false), m_varx(GLINTFILTER_DEFAULT_VARX), 	m_vary(GLINTFILTER_DEFAULT_VARX), 	m_varp(GLINTFILTER_DEFAULT_VARP) 
		{

		}

		/**
		* reset kalman filter
		*/
		void reset() {
			this->m_initialized = false;
		}

		/**
		* push values to kalman filter
		* @param x measured x coordinate
		* @param y measured y coordinate
		*/
		void operator()(float& x, float& y)
		{
			cv::Mat1f p, e;

			if (!this->m_initialized) 
			{
				this->initialize(x, y);
			}
			else
			{
				assert(this->m_initialized);

				this->m_measurement[0] = x;
				this->m_measurement[1] = y;

				// update kalman state
				p = this->m_kf.predict();

				// correct measured values
				e = this->m_kf.correct(cv::Mat1f(2, 1, this->m_measurement));

				x = e(0);
				y = e(1);
			}
		}

		enum Glint
		{
			LEFT_0 = int(0),
			LEFT_1 = int(1),
			LEFT_2 = int(2),
			RIGHT_0 = int(3),
			RIGHT_1 = int(4),
			RIGHT_2 = int(5),
			GAZE = int(6),
		};

		static void correct(
			Glint glint,
			float& x,
			float& y,
			float varx = GLINTFILTER_DEFAULT_VARX,
			float vary = GLINTFILTER_DEFAULT_VARY,
			float varp = GLINTFILTER_DEFAULT_VARP) {
			static GlintFilter gf[7];

			assert(glint >= 0);
			assert(glint <  7);

			if (gf[glint].m_varx != varx ||
				gf[glint].m_vary != vary ||
				gf[glint].m_varp != varp) {
				gf[glint].m_varx = varx;
				gf[glint].m_vary = vary;
				gf[glint].m_varp = varp;
				gf[glint].reset();
			}

			gf[glint](x, y);
		}
	};
}
#endif //KALMAN_H
