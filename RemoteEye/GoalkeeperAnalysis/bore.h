/*
If you use this code please cite the paper:
BORE: Boosted-oriented edge optimization for robust, real time remote pupil center detection
W. Fuhl, S. Eivazi, B. Hosp, A. Eivazi, W. Rosenstiel, E. Kasneci
ACM Symposium on Eye Tracking Research & Applications 2018


Licence and copyright:
It is allowed to use the code and liberaries for research purposes.
The commercial use or the inclusion of the software to commercial products is not allowed.
*/


#ifndef BORE_H_
#define BORE_H_


#include <opencv2/core/core.hpp>

#include <iostream>

#include "elifitter.h"

class BORE {


private:

	ELIFITTER FITTER;

	int START_Y = 10;
	int STOP_Y = 10;
	int START_X = 10;
	int STOP_X = 10;


	int GL_R_STA;
	int GL_R_STE;


	int GL_SZ_R;
	int GL_SZ_O;
	int GL_SZ_D;

	float **idx_ppx;
	float **idx_ppy;
	float ***idx_px;
	float ***idx_py;
	float ***idx_nx;
	float ***idx_ny;


	bool **idx_used;
	float **idx_weight;



	cv::Mat CNM1 = cv::Mat::zeros(100, 100, CV_32FC1);
	cv::Mat CNM2 = cv::Mat::zeros(100, 100, CV_32FC1);


	std::vector<std::vector<cv::Point2i>> INDEXES_P;
	std::vector<std::vector<cv::Point2i>> INDEXES_N;
	std::vector<std::vector<cv::Point2i>> INDEXES_O;



	void m_init();

	void m_fast_idx(cv::Mat &in);

	void m_fast(cv::Mat &in, int st_r, int en_r);

	void m_fast_train(cv::Mat &in, int st_r, int en_r, cv::RotatedRect el, float dist_weight_train);


public:

	void set_AOI(int start_x, int stop_x, int start_y, int stop_y);//start is added to 0 and stop is substracted fromthe input size

	void set_INPUT_SIZE(int SZX, int SZY);

	void m_init_without_file(int min_rad, int max_rad, int step_rad, float step_ori, int dist_diff);

	ellipse_out run_fast(cv::Mat input);

	ellipse_out run_fast_train(cv::Mat input, ellipse_out el, float dist_weight_train);

	void cmp_indexes();

	ellipse_out run_fast_idx(cv::Mat input);

	void m_norm_weight(float prc);

	void m_clear();

	void m_store(std::string path);

	void m_load(std::string path);

	ellipse_out run_fast_iterations(cv::Mat input, int iter);

	ellipse_out run_fast_train_iterations(cv::Mat input, ellipse_out el, float dist_weight_train, int iter);

	ellipse_out run_fast_idx_iterations(cv::Mat input, int iter);
};


#endif