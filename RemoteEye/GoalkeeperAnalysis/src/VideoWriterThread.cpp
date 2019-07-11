#include "VideoWriterThread.h"
#include <iostream>
#include "utils.h"
#include <mutex>



VideoWriterThread::VideoWriterThread(const std::string& filename, int codec, const cv::Size& size, int framerate):
video_writer(filename, codec, framerate, size, false),
stop_flag(false),
filename_(filename)
{
}

VideoWriterThread::~VideoWriterThread()
{
	stop();
}

void VideoWriterThread::enqueueFrame(const QueuedFrame& frame)
{
	std::lock_guard<std::mutex> guard(queue_sync);
	frame_queue.push(frame);	
}

bool VideoWriterThread::start()
{
	if (thread != nullptr)
		stop();
	stop_flag = false;

	if(!video_writer.isOpened())
	{
		qDebug() << "Couldn't open video writer.";
		return false;
	}

	thread.reset(new boost::thread{&VideoWriterThread::run, this});
	return true;
}
void VideoWriterThread::stop()
{
	//if (video_writer.isOpened())
		//video_writer.release();
	stop_flag = true;

	if (thread != nullptr)
	{
		thread->join();
		thread.reset(nullptr);
	}

	if(video_writer.isOpened())
		video_writer.release();
}

void VideoWriterThread::run()
{
	video_info.start_time = getCurrentTime();

	int current_frame_number = 0;
	int emptyFrameCount = 0;

	while (!stop_flag)
	{
		if (frame_queue.empty())
			continue;

		try
		{
			queue_sync.lock();
			const QueuedFrame frame = frame_queue.front();
			frame_queue.pop();
			queue_sync.unlock();
			//TODO: Warum crash.
			if (!frame.frame.empty())
			{
			//	continue;
				video_writer << frame.frame;
				//qDebug() << current_frame_number;
				video_info.frame2time.emplace(current_frame_number, frame.origin_time - video_info.start_time);

				//qDebug() << "Frame captured at: " << frame.origin_time << " computed at " << getCurrentTime();
				current_frame_number++;
			}
			else
			{
				emptyFrameCount++;
				qDebug() << "Frame empty: " << emptyFrameCount;
			}
		}
		catch (cv::Exception& ex)
		{
			std::cerr << "OpenCV Error:\n" << ex.what();
			qDebug() << ex.code;
		}
		catch (std::runtime_error& ex)
		{
			std::cerr << "Runtime Error:\n" << ex.what();
		}
		catch (...)
		{
			std::cerr << "Generic Error";
		}
	}

	// free the rest of the queue
	queue_sync.lock();
	while (!frame_queue.empty())
		frame_queue.pop();
	queue_sync.unlock();

	video_info.end_time = getCurrentTime();	
}

std::string VideoWriterThread::filename() const
{
	return filename_;
}

VideoFrame2Timestamp VideoWriterThread::videoInfo() const
{
	return video_info;
}
