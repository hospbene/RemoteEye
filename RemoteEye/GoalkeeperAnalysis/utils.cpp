#include "utils.h"
#include <boost/filesystem.hpp>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QFile>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

/*
 * Global variables
 */

QElapsedTimer gTimer;


QString gExeDir;
QString gCfgDir;

char gDataSeparator = '\t';
char gDataNewline = '\n';

QMutex gLogMutex;
QFile gLogFile;
QTextStream gLogStream;

std::vector<QString> gLogBuffer;

/*
 * Utility functions
 */

QDebug operator<<(QDebug dbg, const cv::Point2d& p)
{
	dbg.nospace() << "(" << p.x << ", " << p.y << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const cv::Point2f& p)
{
	dbg.nospace() << "(" << p.x << ", " << p.y << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const cv::Point& p)
{
	dbg.nospace() << "(" << p.x << ", " << p.y << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const cv::Point3d& p)
{
	dbg.nospace() << "(" << p.x << ", " << p.y << ", " << p.z << ")";
	return dbg.space();
}


void logExitBanner()
{
	QDateTime utc = QDateTime::currentDateTimeUtc();
	qDebug() << "Exiting\n######################################################################"
		<< "\n# UTC:      " << utc.toString()
		<< "\n# Local:    " << utc.toLocalTime().toString()
		<< "\n######################################################################\n";

	std::cout.flush();
	gLogStream.flush();
}

cv::Point3d estimateMarkerCenter(const std::vector<cv::Point> corners)
{
	cv::Point3d cp(0, 0, 0);
	if (corners.size() != 4)
		return cp;

#ifdef HOMOGRAPHY_ESTIMATION
    // Homography is overkill here
    std::vector<cv::Point> plane;
    plane.push_back(cv::Point(0,0));
    plane.push_back(cv::Point(10,0));
    plane.push_back(cv::Point(10,10));
    plane.push_back(cv::Point(0,10));

    cv::Mat H = cv::findHomography(plane, corners);
    std::vector<cv::Point> planeCenter;
    planeCenter.push_back(cv::Point(5,5));
    std::vector<cv::Point> markerCenter;
    cv::perspectiveTransform(planeCenter, markerCenter, H);
    cp.x = markerCenter[0].x;
    cp.y = markerCenter[0].y;
#else
	double a1 = 0;
	double b1 = 0;
	double a2 = 0;
	double b2 = 0;

	if (corners[2].x - corners[0].x != 0)
		a1 = (corners[2].y - corners[0].y) / (corners[2].x - corners[0].x);
	b1 = corners[2].y - corners[2].x * a1;

	if (corners[3].x - corners[1].x != 0)
		a2 = (corners[3].y - corners[1].y) / (corners[3].x - corners[1].x);
	b2 = corners[3].y - corners[3].x * a2;

	cp.x = (b2 - b1) / (a1 - a2);
	cp.y = a2 * cp.x + b2;
#endif

	return cp;
}

//cv::Point to2D(const cv::Point3d p) { return cv::Point(p.x, p.y); }
//cv::Point3d to3D(const cv::Point p) { return cv::Point3d(p.x, p.y, 0); }

cv::Point2d to2D(const cv::Point3d p) { return cv::Point2d(p.x, p.y); }
cv::Point3d to3D(const cv::Point2d p) { return cv::Point3d(p.x, p.y, 0); }

void ensureDirectoryExists(const std::string& directory)
{
	boost::filesystem::path path(directory);
	boost::filesystem::create_directory(path);
}

void copyFile(const std::string& from, const std::string& to)
{
	boost::filesystem::path from_path(from), to_path(to);
	boost::filesystem::copy(from, to);
}
