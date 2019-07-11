#include "VideoFilePollingThread.h"

#include <chrono>

#include <opencv2/videoio.hpp>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QDebug>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


#include <boost/thread/thread.hpp>
#include <opencv2/videoio/videoio_c.h>

VideoFilePollingThread::VideoFilePollingThread(const std::string& filename, bool loop, VideoCaptureSession::CallbackType callback) :
	VideoFilePollingThread(filename, -1, loop, callback)
{
	
}

VideoFilePollingThread::VideoFilePollingThread(const std::string& filename, double fps, bool loop,
                                   VideoCaptureSession::CallbackType callback) : 
	filename(filename),
    fps(fps), 
	use_true_fps(fps <= 0.01),
    loop(loop),
    callback(callback),
	stop_flag(false),
	capture()
{
	// we need to open this here, as we need to know the framerate and frame size before reading the file
	if (!capture.open(filename))
	{
		qWarning() << "Could not load VideoFilePollingThread for file: " << filename.c_str();
		return;
	}

	frame_size = cv::Size(capture.get(cv::CAP_PROP_FRAME_WIDTH), capture.get(cv::CAP_PROP_FRAME_HEIGHT));
	if (use_true_fps)
	{
		this->fps = capture.get(cv::CAP_PROP_FPS);
	}
	else
	{
		this->fps = fps;
	}
}

VideoFilePollingThread::~VideoFilePollingThread()
{
	
}

void VideoFilePollingThread::stop()
{
	stop_flag = true;
}

void VideoFilePollingThread::operator()()
{
	double target_fps = fps;

	double target_frame_time = 1000. / target_fps;
	unsigned int current_frame_index = 0;
	const unsigned int frame_count = capture.get(cv::CAP_PROP_FRAME_COUNT);

	while (!stop_flag)
	{
		std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();

		cv::Mat frame;
		capture >> frame;

		if(callback)
			callback(frame);
		if(extended_callback)
			extended_callback(frame, capture.get(CV_CAP_PROP_POS_FRAMES), capture.get(CV_CAP_PROP_POS_MSEC));

		frame.release();

		current_frame_index++;

		if (current_frame_index >= frame_count-1)
		{
			if (!loop)
			{
				break;
			}
			else
			{
				capture.set(cv::CAP_PROP_POS_FRAMES, 0);
				current_frame_index = 0;
			}
		}

		std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
		const auto time_taken_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

		double wait_time = target_frame_time - time_taken_ms;
		if (use_true_fps && wait_time > 0)
			boost::this_thread::sleep(boost::posix_time::milliseconds(wait_time));
	}

	capture.release();
	has_finished = true;

	if(end_of_file_callback)
		end_of_file_callback();
}

cv::Size VideoFilePollingThread::videoSize() const
{
	return frame_size;
}

double VideoFilePollingThread::framerate() const
{
	return fps;
}

bool VideoFilePollingThread::hasFinished() const
{
	return has_finished;
}

void VideoFilePollingThread::setEndOfFileCallback(EndOfFileCallback new_callback)
{
	end_of_file_callback = new_callback;
}

void VideoFilePollingThread::setExtendedFrameCallback(ExtendedFrameCallback new_callback)
{
	extended_callback = new_callback;
}

LoadTestingVideoFilePollingThread::LoadTestingVideoFilePollingThread(const std::string& filename, double fps, bool loop,
                                                                     const VideoCaptureSession::CallbackType& callback):
	VideoFilePollingThread(filename, fps, loop, callback) {}

void LoadTestingVideoFilePollingThread::operator()() {
	double target_fps = fps;

	double target_frame_time = 1000. / target_fps;
	unsigned int current_frame_index = 0;
	const unsigned int frame_count = capture.get(cv::CAP_PROP_FRAME_COUNT);
	
	cv::Mat the_frame;
	capture >> the_frame;

	while (!stop_flag)
	{
		std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();

		if (callback)
			callback(the_frame);
		if (extended_callback)
			extended_callback(the_frame, capture.get(CV_CAP_PROP_POS_FRAMES), capture.get(CV_CAP_PROP_POS_MSEC));
		
		current_frame_index++;

		if (current_frame_index >= frame_count - 1)
		{
			if (!loop)
			{
				break;
			}
			else
			{
				capture.set(cv::CAP_PROP_POS_FRAMES, 0);
				current_frame_index = 0;
			}
		}

		std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
		const auto time_taken_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

		double wait_time = target_frame_time - time_taken_ms;
		if (use_true_fps && wait_time > 0)
			boost::this_thread::sleep(boost::posix_time::milliseconds(wait_time));
	}

	capture.release();
	has_finished = true;

	if (end_of_file_callback)
		end_of_file_callback();
}
