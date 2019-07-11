#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <atomic>

#ifndef M_PI
#define M_PI 	 3.14159265358979323846
#endif


#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QElapsedTimer>
#include <QDebug>
#include <QDateTime>
#include <QMutex>
#include <QWidget>
#include <QSettings>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <opencv2/imgproc.hpp>
#include <chrono>

#include "constants.h"
#include "src/IntermediateTypes.h"

/*
 * Global variables
 */
extern QElapsedTimer gTimer;



extern QString gExeDir;
extern QString gCfgDir;
extern char gDataSeparator;
extern char gDataNewline;

extern QList<QPair<QString, QString>> gReferenceList;

/*
 * Utility functions
 */

QDebug operator<<(QDebug dbg, const cv::Point& p);

QDebug operator<<(QDebug dbg, const cv::Point2d& p);


QDebug operator<<(QDebug dbg, const cv::Point2f& p);

QDebug operator<<(QDebug dbg, const cv::Point3d& p);

template <typename T>
double ED(T p1, T p2) { return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)); }


cv::Point3d estimateMarkerCenter(const std::vector<cv::Point> corners);

//void logInitBanner();
//void logExitBanner();
//void logMessages(QtMsgType type, const QMessageLogContext &context, const QString &msg);

// Converstion
//cv::Point to2D(const cv::Point3d p);
//cv::Point3d to3D(const cv::Point p);

cv::Point2d to2D(const cv::Point3d p);
cv::Point3d to3D(const cv::Point2d p);

//QString iniStr(QString str);

//void delay(int thMs);

enum CVFlip { CV_FLIP_BOTH = -1, CV_FLIP_VERTICAL = 0, CV_FLIP_HORIZONTAL = 1, CV_FLIP_NONE = 2 };

Q_DECLARE_METATYPE(enum CVFlip)

#define CV_BLUE 	cv::Scalar(0xff,0xb0,0x00)
#define CV_GREEN 	cv::Scalar(0x03,0xff,0x76)
#define CV_RED 		cv::Scalar(0x00,0x3d,0xff)
#define CV_YELLOW	cv::Scalar(0x00,0xea,0xff)
#define CV_CYAN		cv::Scalar(0xff,0xff,0x18)
#define CV_MAGENT   cv::Scalar(0x81,0x40,0xff)
#define CV_WHITE	cv::Scalar(0xff,0xff,0xff)
#define CV_BLACK	cv::Scalar(0x00,0x00,0x00)

template <typename T>
void set(const QSettings* settings, const QString key, T& v)
{
	if (!settings)
		return;
	QVariant variant = settings->value(key);
	if (variant.isValid())
		v = qvariant_cast<T>(variant);
}



inline FrameTimestamp getCurrentTime()
{
	FrameTimestamp now = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	return now;
}


void ensureDirectoryExists(const std::string& directory);
void copyFile(const std::string& from, const std::string& to);



inline double deg_to_rad(double a)
{
	return a * 3.141592653589793 / 180.;
}

inline double rad_to_deg(double a)
{
	return (a * 180.) / 3.141592653589793;
}




#endif // UTILS_H
