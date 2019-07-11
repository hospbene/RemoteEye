#pragma once

#include <boost\circular_buffer.hpp>
#include <opencv.hpp>
#include<iostream>
#include<stdio.h>
#include <numeric>
#include <QDebug>


class MAFilter
{
public:
	cv::Point2f MAFilterTotal = { 0.0f,0.0f };
	//MAFilter(int min_range, int max_range, int windowDepth, bool outlier, bool gauss, short MAX_GAUSS_FACTOR);
	MAFilter();
	MAFilter(int windowDepth, bool outlier, short GAUSS_FACTOR);
	virtual ~MAFilter();

	void isOutlier(cv::Point2f originalGaze);
	cv::Point2f calcMean();
	cv::Point2f calcStdDev();
	void calcTotal();
	void addSample(cv::Point2f newSample);
	cv::Point2f movingAveragesFilter(cv::Point2f originalGaze);


private:
	int MAFilterWindow;
	int min_range, max_range;
	boost::circular_buffer<cv::Point2f> MAFilterSamples;
	bool USE_OUTLIER_DETECTION = false;
	bool USE_GAUSS = false;
	short MAX_GAUSS_FACTOR = 2;
	bool useKalman = false;

};

