#include "BHOKalmanFilter.h"
#include "qdebug.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/tracking.hpp"


#define drawCross( center, color, d )                                 \
line( img, cv::Point( center.x - d, center.y - d ), cv::Point( center.x + d, center.y + d ), color, 2, CV_AA, 0); \
line( img, cv::Point( center.x + d, center.y - d ), cv::Point( center.x - d, center.y + d ), color, 2, CV_AA, 0 )

BHOKalmanFilter::BHOKalmanFilter()
{

	//this->m_kf = cv::KalmanFilter(4, 2, 0);


	this->m_kf = cv::KalmanFilter(
		GLINTFILTER_DYNAMIC_PARAMETERS,
		GLINTFILTER_MEASURED_PARAMETERS,
		GLINTFILTER_CONTROL_PARAMETERS);



	cv::Point2f mousePos;
	img = cv::Mat(600, 800, CV_8UC3);

	this->initialize(cv::Point(50.0f, 100.0f));

}



void BHOKalmanFilter::initialize(cv::Point2f startPos)
{
	
	/*
	// intialization of KF...
	this->m_kf.transitionMatrix = (cv::Mat_<float>(4, 4) << 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1);
	this->measurement = cv::Mat_<float>(2, 1);
	this->measurement.setTo(cv::Scalar(0));

	cv::setIdentity(m_kf.measurementMatrix);
	cv::setIdentity(m_kf.processNoiseCov, cv::Scalar::all(1e-4));
	cv::setIdentity(m_kf.measurementNoiseCov, cv::Scalar::all(10));
	cv::setIdentity(m_kf.errorCovPost, cv::Scalar::all(.1));
	*/
	this->mousev.clear();
	this->kalmanv.clear();
	


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
	this->m_state_pre_matrix[0] = 0.0f;	// x
	this->m_state_pre_matrix[1] = 0.0f;	// y
	this->m_state_pre_matrix[2] = 0.0f;
	this->m_state_pre_matrix[3] = 0.0f;
	this->m_kf.statePre = cv::Mat1f(4, 1, this->m_state_pre_matrix);

	// initialize measurement
	// [ 0
	//   0 ]
	this->m_measurement[0] = 0.0f;
	this->m_measurement[1] = 0.0f;


	
	
}



cv::Point2f BHOKalmanFilter::addSample(cv::Point2f newSample)
{
	qDebug() << "NewPosition: " << newSample.x << " " << newSample.y;
	cv::Mat1f p, e;

	// First predict, to update the internal statePre variable
	//cv::Mat prediction = this->m_kf.predict();
	//cv::Point predictPt(prediction.at<float>(0), prediction.at<float>(1));

	// Get mouse point
	//this->m_measurement(0) = newPosition.x;
	//this->m_measurement(1) = newPosition.y;

	// The update phase 
	//cv::Mat estimated = this->m_kf.correct(this->m_measurement);

	//cv::Point statePt(estimated.at<float>(0), estimated.at<float>(1));
	//cv::Point measPt(this->m_measurement(0), this->m_measurement(1));
	
	//this->m_measurement[0] = newSample.x;
	//this->m_measurement[1] = newSample.y;

	this->m_measurement[0] = newSample.x;
	this->m_measurement[1] = newSample.y;

	// update kalman state
	p = this->m_kf.predict();

	// correct measured values
	e = this->m_kf.correct(cv::Mat1f(2, 1, this->m_measurement));

	//x = e(0);
	//y = e(1);

	cv::Point2f newPoint = cv::Point2f(e(0), e(1));
	
	/*
	// plot points
	if(img.cols >0 && img.rows >0)
	cv::imshow("mouse kalman", img);
	img = cv::Scalar::all(0);

	this->mousev.push_back(measPt);
	this->kalmanv.push_back(statePt);

	drawCross(statePt, cv::Scalar(0, 0, 255), 5);
	drawCross(measPt, cv::Scalar(0, 255, 0), 5);

	
	for (int i = 0; i < this->mousev.size() - 1; i++)
		cv::line(img, this->mousev[i], this->mousev[i + 1], cv::Scalar(0, 0, 255), 1);

	for (int i = 0; i < this->kalmanv.size() - 1; i++)
		cv::line(img, this->kalmanv[i], this->kalmanv[i + 1], cv::Scalar(0, 255, 0), 1);
		
	cv::waitKey(1);*/
	//return statePt;

	qDebug() << "Estimated: " << newPoint.x << " " << newPoint.y;
	return newPoint;
	
}


BHOKalmanFilter::~BHOKalmanFilter()
{
}
