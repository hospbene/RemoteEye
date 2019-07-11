#ifndef ALGO_FWD_H_
#define ALGO_FWD_H_

// declarations for the algo.h header pending splitting that up.

#include <opencv2/core.hpp>


namespace ELSE {
	void m_init_1();
	void m_load_1(std::string path);
	void init_mats_1();
	void m_clear_1();
	void cmp_indexes_1();
	void m_fast_idx_1(cv::Mat& in);
	cv::RotatedRect run_fast_idx_1(cv::Mat input);
	void m_init_2();
	void m_load_2(std::string path);
	void init_mats_2();
	void m_clear_2();
	void cmp_indexes_2();
	void m_fast_idx_2(cv::Mat& in);
	cv::RotatedRect run_fast_idx_2(cv::Mat input);
}


#endif