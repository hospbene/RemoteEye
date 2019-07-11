#include "InputFeaturesDisplay.h"


#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QDesktopWidget>
#include <QApplication>
#include <QPainter>
#include <opencv2/imgproc.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ProfilingMonitor.h"
#include "OutputMethods.h"
#include <mutex>


class NewDataEvent : public QEvent
{
public:
	static const QEvent::Type my_type = static_cast<QEvent::Type>(QEvent::User + 45);
	explicit NewDataEvent() : QEvent(my_type) {}
};


InputFeaturesDisplay::InputFeaturesDisplay(double max_framerate, QWidget* parent) :
	QWidget(parent),
	max_framerate(max_framerate),
	last_render(0),
	converter_thread_instance(new InputFeaturesConverterThread([this](QPixmap new_image, cv::Mat new_mat) {this->convertedImageCallback(new_image, new_mat); })),
	converter_thread(&InputFeaturesConverterThread::run, converter_thread_instance)
{
	//setWindowFlags(Qt::FramelessWindowHint);
	setStyleSheet("QWidget{ background: #444444 }");
	this->setWindowTitle(QString("Input Features [@").append(QString::number(max_framerate)).append(" fps]"));
}

InputFeaturesDisplay::~InputFeaturesDisplay()
{
	converter_thread.interrupt();
	emit ended();
	delete converter_thread_instance;
}

void InputFeaturesDisplay::paintEvent(QPaintEvent* event)
{
	std::lock_guard<std::mutex> lock(paint_lock);
	last_render = getCurrentTime();
	ProfilingSection ps;
	QPainter painter(this);
	
	painter.drawPixmap(QRectF(QPoint(0, 0), QSizeF(last_image.width(), last_image.height())),
		last_image, QRectF(QPoint(0, 0), QSizeF(last_image.width(), last_image.height())));
	//gProfilingMonitor.addTiming("input_feature_display paint", ps);
	//gProfilingMonitor.addTiming("input_feature_display freq", ps, true);
}

bool InputFeaturesDisplay::event(QEvent* event)
{
	if (event->type() == QEvent::KeyPress || event->type() == QEvent::Close) {
		this->close();
		emit ended();
		return true;
	}
	else if(event->type() == NewDataEvent::my_type)
	{
		if (last_image.width() != this->width() || last_image.height() != this->height())
		{
			//TODO: Check how often this is called
			resize(last_image.width(), last_image.height());
		}
		this->repaint();
	}
	return QWidget::event(event);
}

void InputFeaturesDisplay::convertedImageCallback(QPixmap new_image, cv::Mat new_mat)
{
	{
		std::lock_guard<std::mutex> lock(paint_lock);
		last_mat = new_mat;
		last_image = new_image;
	}
	
	QApplication::postEvent(this, new NewDataEvent(), Qt::HighEventPriority);
}

void InputFeaturesDisplay::imageAvailable(const cv::Mat& image, const DetectedFeatures& features)
{
	FrameTimestamp current_time = getCurrentTime();
	if (current_time - last_render < (1. / max_framerate))
		return;

	InputFeaturesConverterThreadInput cvt;
	cvt.frame = image;
	cvt.features = features;
	converter_thread_instance->queue(cvt);
}

InputFeaturesConverterThread::InputFeaturesConverterThread(DataCallback callback):
callback(callback)
{
	
}

void InputFeaturesConverterThread::queue(InputFeaturesConverterThreadInput& input)
{
	std::lock_guard<std::mutex> lock(condition_new_data_lock);
	next_input = input;
	new_input_conversion_desired = true;
	condition_new_data.notify_one();
}
void InputFeaturesConverterThread::run()
{
	while(true)
	{
		std::unique_lock<std::mutex> lock(condition_new_data_lock);
		condition_new_data.wait(lock);
		bool state = new_input_conversion_desired;
		new_input_conversion_desired = false;
		InputFeaturesConverterThreadInput input_to_process = next_input;
		next_input = {};
		lock.unlock();

		if (!state)
			continue;

		if (boost::this_thread::interruption_requested())
			break;

		cv::Mat last_mat = input_to_process.frame.clone();
		cv::cvtColor(last_mat, last_mat, cv::COLOR_GRAY2RGB);
		drawFeatures(last_mat, input_to_process.features);


		QPixmap pixmap = QPixmap::fromImage(QImage(last_mat.data, last_mat.cols, last_mat.rows, last_mat.step, QImage::Format_RGB888));
		
		if (boost::this_thread::interruption_requested())
			break;
		callback(pixmap, last_mat);
	}
}


