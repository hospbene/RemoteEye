#include "CalibrationDialog.h"
#include "../GeneratedFiles/ui_CalibrationDialog.h"
#include "IntermediateTypes.h"

#include <mutex>
#include <condition_variable>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#pragma warning(push, 0)
#include <QEvent>
#pragma warning(pop)

#include <cameraCalibration.h>
#include "utils.h"

#include "CalibrationTrainer.h"

// condition variable to make sure that no matter the usage of the main thread (ie qt's event loop) or the time taken to render
// the 500ms wait after the image is actually rendered is true and accurate.
std::condition_variable condition_display;
std::mutex condition_lock;
bool image_was_displayed = false;


class DisplayCalibrationImageEvent : public QEvent
{
public:
	static const QEvent::Type my_type = static_cast<QEvent::Type>(QEvent::User + 1);

	QImage image_data;

	explicit DisplayCalibrationImageEvent(QImage image_data) : QEvent(my_type), image_data(image_data){}
};


class CalibrationFinishedEvent : public QEvent
{
public:
	static const QEvent::Type my_type = static_cast<QEvent::Type>(QEvent::User + 2);
	explicit CalibrationFinishedEvent() : QEvent(my_type){}
};

/// Signals that collecting data for the current point should be stopped (this doesn't mean that a new point should be displayed)
class StopCollectingCalibrationDataEvent : public QEvent
{
public:
	static const QEvent::Type my_type = static_cast<QEvent::Type>(QEvent::User + 3);
	explicit StopCollectingCalibrationDataEvent() : QEvent(my_type) {}
};

/// Signals that collecting data for the current point should be started
class StartCollectingCalibrationDataEvent : public QEvent
{
public:
	std::pair<float, float> point;
	static const QEvent::Type my_type = static_cast<QEvent::Type>(QEvent::User + 4);
	explicit StartCollectingCalibrationDataEvent(std::pair<float, float> point) : QEvent(my_type), point(point) {}
};

QImage cvMatToQimage(cv::Mat mat)
{
	return QImage(static_cast<unsigned char*>(mat.data), mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
}

class CalibrationSequenceRunner
{
public:
	explicit CalibrationSequenceRunner(const std::vector<std::pair<double, double>>& calibration_point, CalibrationDialog* const dialog,
		const unsigned int width, const unsigned int height):
	all_calibration_points(calibration_point),
	dialog(dialog),
	width(width),
	height(height)
	{
		
	}

	/// Posts the image to the queue of the main thread to send off for rendering
	/// and blockingly waits until the image has actually been rendered there.
	void effectDisplay(QImage& image) const
	{
		boost::this_thread::interruption_point();
		auto event = new DisplayCalibrationImageEvent(image);
		QCoreApplication::postEvent(dialog, event, Qt::NormalEventPriority);
		std::unique_lock<std::mutex> lock(condition_lock);
		condition_display.wait(lock, [] {return image_was_displayed; });
		image_was_displayed = false;
		lock.unlock();
	}
	

	/// displays an image that shows only the given set of points, waits, collects, waits again.
	/// \param point The desired true point of gaze on the screen
	/// \param points The set of markers to draw.
	void displaySequenceElement(const std::pair<double, double> point, const std::vector<std::pair<double, double>>& points, 
		unsigned int wait_before=600, unsigned int wait_collect=500, unsigned int wait_after=500) const
	{
		// first show a grayed image for 1 sec for the user to see where to look next
		cv::Mat mat(cv::Size(width, height), CV_8UC3);
		mat = cv::Scalar(255, 255, 255);
		
		for(const auto& point : points){
			cv::circle(mat, cv::Point(point.first, point.second), 20, cv::Scalar(80, 80, 80), CV_FILLED, 8, 0);
			cv::drawMarker(mat, cv::Point(point.first, point.second), cv::Scalar(255, 255, 255), cv::MARKER_CROSS, 40, 2); // 1     0,0
		}

		QImage image = cvMatToQimage(mat);
		effectDisplay(image);

		boost::this_thread::sleep(boost::posix_time::milliseconds(wait_before));
		auto start_event = new StartCollectingCalibrationDataEvent(points[0]);
		QCoreApplication::postEvent(dialog, start_event, Qt::NormalEventPriority);

		boost::this_thread::sleep(boost::posix_time::milliseconds(wait_collect));

		auto stop_event = new StopCollectingCalibrationDataEvent();
		QCoreApplication::postEvent(dialog, stop_event, Qt::NormalEventPriority);
		boost::this_thread::sleep(boost::posix_time::milliseconds(wait_after));
	}

	void operator() (){
		try{
			// Initial Instruction window
			cv::Mat all_markers(cv::Size(width, height), CV_8UC3);
			all_markers = cv::Scalar(255, 255, 255);
			cv::putText(
				all_markers,
				"To calibrate focus on the different spots shown below sequentially. Please focus on the center of the cross as long as it is shown. Press any key to start calibration. ",
				cv::Point(20, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);

			// Print the calibrationpoint  on screen with cross and circle
			for (unsigned int i = 0; i < all_calibration_points.size(); i++) 
			{	
				const auto& point = all_calibration_points[i];
				cv::drawMarker(all_markers, cv::Point(point.first, point.second), cv::Scalar(0, 0, 0), MARKER_CROSS, 40, 2);
				cv::circle(all_markers, cv::Point(point.first, point.second), 20, cv::Scalar(0, 0, 0), 1, 8, 0);
				cv::putText(all_markers, to_string(i + 1), cv::Point2f(point.first + 20, point.second - 20), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 0, 0), 1);
			}

			QImage image = cvMatToQimage(all_markers);
			effectDisplay(image);
			//TODO: wait here once implementation is finished
			boost::this_thread::sleep(boost::posix_time::milliseconds(0));

		
			// display all points individually
			for (const auto& all_calibration_point : all_calibration_points)
			{
				displaySequenceElement(all_calibration_point, {all_calibration_point});
			}


			QCoreApplication::postEvent(dialog, new CalibrationFinishedEvent(), Qt::NormalEventPriority);
		}
		catch(boost::thread_interrupted& e)
		{
			return;
		}
	}
		
private:
	const std::vector<std::pair<double, double>> all_calibration_points;
	CalibrationDialog* const dialog;
	const unsigned int width, height;
};


CalibrationDialog::CalibrationDialog(const std::vector<std::pair<double, double>>& calibration_points, 
	std::pair<unsigned int, unsigned int> size, 
	CalibrationTrainer* trainer, 
	QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalibrationDialog),
	runner(new CalibrationSequenceRunner(calibration_points, this, size.first, size.second)),	
	thread_sequence(&CalibrationSequenceRunner::operator(), runner.get()),
	training(trainer),
	has_been_consumed(false)
{
	GA_ASSERT(training != nullptr);

    ui->setupUi(this);
	// makes sure the window is at its proper position if the taskbar is at the top.
	move(0, 0);
}

CalibrationDialog::~CalibrationDialog()
{
	thread_sequence.interrupt();
    delete ui;
}

bool CalibrationDialog::event(QEvent* event)
{
	if(event->type() == DisplayCalibrationImageEvent::my_type)
	{
		auto* dcie = dynamic_cast<DisplayCalibrationImageEvent*>(event);

		ui->calibrationImageLabel->setScaledContents(false);
		ui->calibrationImageLabel->setFixedSize(dcie->image_data.size());
		ui->calibrationImageLabel->setPixmap(QPixmap::fromImage(dcie->image_data));
		this->repaint();

		{
			std::lock_guard<std::mutex> lock(condition_lock);
			image_was_displayed = true;
		}

		condition_display.notify_one();
		return true;
	}
	else if(event->type() == CalibrationFinishedEvent::my_type)
	{
		this->close();
		
		emit calibrationFinished();
		return true;
	}
	else if(event->type() == StartCollectingCalibrationDataEvent::my_type)
	{
		// discard any data that has been collected inbetween points
		current_point = (dynamic_cast<StartCollectingCalibrationDataEvent*>(event))->point;
		start_of_current_point = getCurrentTime();
		std::lock_guard<std::mutex> guard(data_buffer_mutex);
		data_buffer.clear();
	}
	else if (event->type() == StopCollectingCalibrationDataEvent::my_type)
	{
		std::lock_guard<std::mutex> guard(data_buffer_mutex);
		CalibrationPointData this_point_data;
		this_point_data.position = cv::Point2f(current_point.first, current_point.second);
		this_point_data.data.insert(this_point_data.data.end(), data_buffer.begin(), data_buffer.end());
		this_point_data.end_time = getCurrentTime();
		this_point_data.start_time = start_of_current_point;
		collected_data.calibration_points.push_back(this_point_data);
		data_buffer.clear();
	}

	return QDialog::event(event);
}

void CalibrationDialog::receiveEyeFeatures(PupilGlintCombiInstance features)
{
	std::lock_guard<std::mutex> guard(data_buffer_mutex);
	data_buffer.push_back(features);
}

void CalibrationDialog::receiveRawImage(QueuedFrame& frame)
{
	if (has_been_consumed)
		return;

	std::lock_guard<std::mutex> guard(data_buffer_mutex);
	training->receiveData(frame);
}

bool CalibrationDialog::needsDetectedFeatures() const
{
	return !training->overwritesFeatureDetection();
}


CalibrationData CalibrationDialog::getCalibratedData()
{
	GA_ASSERT(!has_been_consumed && "Calibration Dialog Results can only be calculated once. Subsequent retrieval of the data is not possible.");
	training->enrich(collected_data);
	has_been_consumed = true;

	return collected_data;
}
