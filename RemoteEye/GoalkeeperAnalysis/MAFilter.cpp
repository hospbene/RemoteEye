#include "MAFilter.h"
#include "opencv2/video/tracking.hpp"


MAFilter::MAFilter(int windowDepth, bool outlier, short GAUSS_FACTOR)
{
	MAFilterWindow = windowDepth;
	this->MAX_GAUSS_FACTOR = GAUSS_FACTOR;
	USE_OUTLIER_DETECTION = outlier;
	MAFilterSamples = boost::circular_buffer<cv::Point2f>(MAFilterWindow, cv::Point2f(0.0f, 0.0f));
	this->min_range = min_range;
	this->max_range = max_range;
	

}

MAFilter::~MAFilter()
{
}



MAFilter::MAFilter()
{
}

// TODO:_ Use proper outlier detection not just fixed values
cv::Point2f MAFilter::movingAveragesFilter(cv::Point2f originalGaze)
{
	
	// use only valid settings
	if(originalGaze.x > 0 && originalGaze.y > 0)
	{
		if(USE_OUTLIER_DETECTION)
		{
			this->isOutlier(originalGaze);
		}
		else
		{

			this->addSample(originalGaze);
		}
	
								
	}

	// 2. neue Summe berechnen
	this->calcTotal();
	cv::Point2f newGazePoint(this->MAFilterTotal.x / this->MAFilterWindow, this->MAFilterTotal.y / this->MAFilterWindow);	// 3. neuer mittelwert berechnen
	
	
	return newGazePoint;
}

// TODO: optimize outlier detection
void MAFilter::isOutlier(cv::Point2f originalGaze)
{

	cv::Point2f stddev = calcStdDev();
	cv::Point2f mean = calcMean();

	//this->addSample(originalGaze);
		
	if ((originalGaze.x > (mean.x + (this->MAX_GAUSS_FACTOR *stddev.x)))
		|| (originalGaze.x < (mean.x - (this->MAX_GAUSS_FACTOR * stddev.x)))
		|| (originalGaze.y > (mean.y + (this->MAX_GAUSS_FACTOR*stddev.y)))
		|| (originalGaze.y < (mean.y - (this->MAX_GAUSS_FACTOR * stddev.y))))
	{
		
		this->addSample(originalGaze);

	}
	else 
	{
					
		this->addSample(mean);


						
	}
	
	

}

cv::Point2f MAFilter::calcMean()
{
	boost::circular_buffer<cv::Point2f>::iterator it = MAFilterSamples.begin();
	
	double meanX, meanY;

	// 1. Summiere alle quadrierten Absolutwerte der Differenzen zwischen Sample und Mean
	int i = 0;
	for (it = MAFilterSamples.begin(); it != MAFilterSamples.end(); it++, i++)
	{

		meanX += MAFilterSamples[i].x;
		meanY += MAFilterSamples[i].y;
	}

	cv::Point2f mittelWert(meanX / MAFilterSamples.size(), meanY / MAFilterSamples.size());

	return mittelWert;
}

cv::Point2f MAFilter::calcStdDev()
{
		
	cv::Point2f mittelWert(MAFilterTotal.x / MAFilterSamples.size(), MAFilterTotal.y / MAFilterSamples.size());	
	double summedStddevX, summedStddevY;
	double stddevX, stddevY;

	boost::circular_buffer<cv::Point2f>::iterator it = MAFilterSamples.begin();


	// 1. Summiere alle quadrierten Absolutwerte der Differenzen zwischen Sample und Mean
	int i = 0;
	for (it = MAFilterSamples.begin(); it != MAFilterSamples.end(); it++, i++)
	{

		summedStddevX += pow(abs(MAFilterSamples[i].x - mittelWert.x), 2);
		summedStddevY += pow(MAFilterSamples[i].y - mittelWert.y, 2);
	}

	// 2. Teile die Summen durch die Anzahl der Samples
	// und ziehe die Wurzel daraus
	stddevX = sqrt(summedStddevX / MAFilterSamples.size());
	stddevY = sqrt(summedStddevY / MAFilterSamples.size());


	cv::Point2f stddev(stddevX, stddevY);

	return stddev;
}

// calculate the sum of all elements.
void MAFilter::calcTotal()
{
	cv::Point2f zero(0.0f, 0.0f);
	this->MAFilterTotal = std::accumulate(MAFilterSamples.begin(), MAFilterSamples.end(), zero);
}

// DESCR: Adds a new Sample to the ring buffer and if full kicks the oldest sample out
void MAFilter::addSample(cv::Point2f newSample)
{
	MAFilterSamples.push_back(newSample);
}
