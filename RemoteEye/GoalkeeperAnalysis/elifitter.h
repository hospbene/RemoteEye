/*
If you use this code please cite the paper:
BORE: Boosted-oriented edge optimization for robust, real time remote pupil center detection
W. Fuhl, S. Eivazi, B. Hosp, A. Eivazi, W. Rosenstiel, E. Kasneci
ACM Symposium on Eye Tracking Research & Applications 2018


Licence and copyright:
It is allowed to use the code and liberaries for research purposes. 
The commercial use or the inclusion of the software to commercial products is not allowed.
*/


#pragma once



#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#include <iostream>
#include <fstream>


struct ellipse_out {
	cv::RotatedRect eli;
	bool valid;
};


//FITTER

class ELIFITTER {

private:

	cv::Mat CNM1 = cv::Mat::zeros(100, 100, CV_32FC1);
	cv::Mat CNM2 = cv::Mat::zeros(100, 100, CV_32FC1);

	int GL_R_STA_ELI;
	int GL_R_STE_ELI;


	int GL_SZ_R_ELI;
	int GL_SZ_O_ELI;
	int GL_SZ_D_ELI;

	float **idx_ppx_ELI;
	float **idx_ppy_ELI;
	float ***idx_px_ELI;
	float ***idx_py_ELI;
	float ***idx_nx_ELI;
	float ***idx_ny_ELI;


	bool **idx_used_ELI;
	float **idx_weight_ELI;


	std::vector<int> rad_idx;
	std::vector<cv::Point2i> rads_to_idx;


	void m_init_ELI();

public:

	void set_INPUT_SIZE(int SZX, int SZY);


	void m_store(std::string path);

	void m_load(std::string path);
	
	void m_init_without_file_eli(int min_rad, int max_rad, int step_rad, float step_ori, int dist_diff);

	void m_clear_eli();

	void m_fast_eli(cv::Mat &in, ellipse_out &pos);


};
















