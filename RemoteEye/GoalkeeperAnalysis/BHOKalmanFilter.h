#pragma once
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/tracking.hpp"

#define GLINTFILTER_DYNAMIC_PARAMETERS  int(4) // [x,y,dx,dy]
#define GLINTFILTER_MEASURED_PARAMETERS int(2) // [x,y]
#define GLINTFILTER_CONTROL_PARAMETERS  int(0) // []

#define GLINTFILTER_DEFAULT_VARX float(0.1000f)
#define GLINTFILTER_DEFAULT_VARY float(0.1000f)
#define GLINTFILTER_DEFAULT_VARP float(0.0001f)


class BHOKalmanFilter
{
public:
	

	// KF states
	float m_transition_matrix[GLINTFILTER_DYNAMIC_PARAMETERS*GLINTFILTER_DYNAMIC_PARAMETERS];
	float m_measurement_matrix[GLINTFILTER_MEASURED_PARAMETERS*GLINTFILTER_DYNAMIC_PARAMETERS];
	float m_processs_noise_matrix[GLINTFILTER_DYNAMIC_PARAMETERS*GLINTFILTER_DYNAMIC_PARAMETERS];
	float m_measurement_noise_matrix[GLINTFILTER_MEASURED_PARAMETERS*GLINTFILTER_MEASURED_PARAMETERS];
	float m_post_error_matrix[GLINTFILTER_MEASURED_PARAMETERS*GLINTFILTER_MEASURED_PARAMETERS];
	float m_state_pre_matrix[GLINTFILTER_DYNAMIC_PARAMETERS];
	float m_measurement[GLINTFILTER_MEASURED_PARAMETERS];




	//indicates if kalman filter is initialized and ready to use
	bool m_initialized;

	// expected error in x direction
	float m_varx;

	// expected error in y direction
	float m_vary;


	float m_varp;

	


	cv::Mat img;
	std::vector<cv::Point2f> mousev, kalmanv;
	cv::KalmanFilter m_kf;
	//cv::Mat_<float> measurement;
	BHOKalmanFilter();
	void initialize(cv::Point2f startPos);
	
	cv::Point2f addSample(cv::Point2f newPosition);
	~BHOKalmanFilter();
};

