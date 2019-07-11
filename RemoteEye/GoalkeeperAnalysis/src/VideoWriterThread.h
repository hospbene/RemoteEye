#ifndef EYE_VIDEO_WRITER_H_
#define EYE_VIDEO_WRITER_H_

#include "GoalkeeperAnalysis.h"
#include <opencv2/videoio.hpp>
#include <boost/thread/thread_only.hpp>
#include <mutex>
#include <queue>

class VideoWriterThread
{
public:
	explicit VideoWriterThread(const std::string& filename, int codec, const cv::Size& size, int framerate);
	VideoWriterThread(const VideoWriterThread& other) = delete;
	VideoWriterThread& operator=(const VideoWriterThread&) = delete;
	~VideoWriterThread();

	void enqueueFrame(const QueuedFrame& frame);
	/// Attempts to start capture.
	/// \return If false is returned, starting was not possible, but no thread has been started as a result.
	bool start();
	void stop();

	void run();

	std::string filename() const;
	VideoFrame2Timestamp videoInfo() const;

private:
	std::unique_ptr<boost::thread> thread;
	VideoFrame2Timestamp video_info;
	std::atomic<bool> stop_flag;
	std::mutex queue_sync;
	std::queue<QueuedFrame> frame_queue;
	cv::VideoWriter video_writer;
	const std::string filename_;
};


#endif