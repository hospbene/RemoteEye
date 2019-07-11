#ifndef VIDEO_CAPTURE_SESSION_H_
#define VIDEO_CAPTURE_SESSION_H_

#include <functional>
#include <memory>

#include <opencv2/core.hpp>
#include <boost/thread/thread_only.hpp>

struct RunningCaptureThread;

class VideoPollingBackend
{
public:
	virtual ~VideoPollingBackend() = default;
	virtual void operator()() = 0;
	
	/// stops capture as soon as possible
	virtual void stop() = 0;
};

/// Regularly polls the given polling backend for new input in a separate thread.
class VideoCaptureSession
{
public:
	typedef std::function<void(const cv::Mat&)> CallbackType;

	/// Takes ownership of the given polling backend
	explicit VideoCaptureSession(VideoPollingBackend* backend);
	~VideoCaptureSession();


	/// \brief 
	/// \param callback The callback function. Executed on the capture thread, so exit quickly.
	/// \return 
	bool startCapture();
	void stopCapture();

	const VideoPollingBackend* getBackend() const;
private:
	VideoPollingBackend* polling_functor;
	/// We never want capture to run detached from this object, so this comes with a deleter that 
	/// ensures that on deletion here the capture is signalled to stop, the thread is joined and
	/// then deleted. As a result, reset/delete blocks until this is possible.
	std::unique_ptr<RunningCaptureThread, void(*)(RunningCaptureThread*)> polling_thread;
};

struct RunningCaptureThread
{
	boost::thread* thread;
	VideoPollingBackend* functor;
};


#endif