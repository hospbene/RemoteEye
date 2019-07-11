#ifndef VIDEO_FILE_POLLING_THREAD_H_
#define VIDEO_FILE_POLLING_THREAD_H_

#include <atomic>
#include "VideoCaptureSession.h"
#include <opencv2/videoio.hpp>


/// Alternative insert (mainly for testing when the uEye camera is not available).
///
/// Uses OpenCV video capture to read from the given file.
class VideoFilePollingThread : public VideoPollingBackend
{
public:
	typedef std::function<void()> EndOfFileCallback;
	typedef std::function<void(const cv::Mat&, unsigned int, double)> ExtendedFrameCallback;

	/// \brief Will call callback with video data at the designated playback speed of the video (if possible).
	/// \param filename The file to read from (or any pattern that is acceptable to cv::VideoCapture)
	/// \param loop Whether to loop the file endlessly until stopped explicitly.
	/// \param callback A callback that receives the video data.
	explicit VideoFilePollingThread(const std::string& filename, bool loop, VideoCaptureSession::CallbackType callback);
	/// \brief Allows to override the FPS at which the video is served.
	/// \param filename The file to read from (or any pattern that is acceptable to cv::VideoCapture)
	/// \param loop Whether to loop the file endlessly until stopped explicitly.
	/// \param callback A callback that receives the video data.
	explicit VideoFilePollingThread(const std::string& filename, double fps, bool loop, VideoCaptureSession::CallbackType callback);

	virtual ~VideoFilePollingThread();

	void stop() override;
	void operator()() override;
	cv::Size videoSize() const;
	double framerate() const;
	bool hasFinished() const;
	/// \brief Sets the end of file callback, overriding the previously registered callback.
	void setEndOfFileCallback(EndOfFileCallback new_callback);
	void setExtendedFrameCallback(ExtendedFrameCallback new_callback);

protected:
	const std::string filename;
	double fps;
	const bool use_true_fps;
	const bool loop;
	VideoCaptureSession::CallbackType callback;
	EndOfFileCallback end_of_file_callback;
	ExtendedFrameCallback extended_callback;
	std::atomic<bool> stop_flag;
	std::atomic<bool> has_finished;
	cv::VideoCapture capture;
	cv::Size frame_size;
};


/// This does not read the video file from disk, it just delivers the first image over and over from memory
class LoadTestingVideoFilePollingThread : public VideoFilePollingThread
{
public:
	LoadTestingVideoFilePollingThread(const std::string& filename, double fps, bool loop,
	                                  const VideoCaptureSession::CallbackType& callback);
	void operator()() override;
};
#endif