#ifndef INPUT_FEATURES_DISPLAY_H_
#define INPUT_FEATURES_DISPLAY_H_


#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QWidget>
#include "GazeEstimationAdapters.hpp"
#include <memory>
#include <boost/thread/thread_only.hpp>
#include <mutex>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


struct InputFeaturesConverterThreadInput
{
	cv::Mat frame;
	DetectedFeatures features;
};

class InputFeaturesConverterThread
{
public:
	typedef std::function<void(QPixmap, cv::Mat)> DataCallback;

	explicit InputFeaturesConverterThread(DataCallback callback);

	void queue(InputFeaturesConverterThreadInput& input);
	void run();

private:
	InputFeaturesConverterThreadInput next_input;
	DataCallback callback;
	std::condition_variable condition_new_data;
	std::mutex condition_new_data_lock;
	bool new_input_conversion_desired = false;
};

class InputFeaturesDisplay : public QWidget
{
Q_OBJECT
public:
	explicit InputFeaturesDisplay(double max_framerate, QWidget* parent = nullptr);
	~InputFeaturesDisplay();

protected:
	void paintEvent(QPaintEvent* event) override;
	bool event(QEvent* event) override;
	void convertedImageCallback(QPixmap new_image, cv::Mat new_mat);

public slots:
	void imageAvailable(const cv::Mat& image, const DetectedFeatures& features);

signals :
	/// Signals that the user is no longer interested in the output of this. Can be emitted multiple times
	/// over the lifetime of this object.
	void ended();

private:
	FrameTimestamp last_render;
	QPixmap last_image;
	cv::Mat last_mat;

	double max_framerate;
	std::mutex paint_lock; // can't update the image data during paint

	InputFeaturesConverterThread* converter_thread_instance;
	boost::thread converter_thread;
};


#endif
