#include "VideoCaptureSession.h"
#include <boost/bind/bind.hpp>
#include <boost/thread/thread_only.hpp>



void deleteCaptureThread(RunningCaptureThread* running_capture)
{
	if (running_capture == nullptr)
		return;

	running_capture->functor->stop();
	running_capture->thread->join();
	delete running_capture->thread;
	delete running_capture->functor;
	delete running_capture;
}

VideoCaptureSession::VideoCaptureSession(VideoPollingBackend* functor) :
	polling_functor(functor),
	polling_thread(nullptr, &deleteCaptureThread) { }

VideoCaptureSession::~VideoCaptureSession()
{
	polling_thread.reset(nullptr);
}

bool VideoCaptureSession::startCapture()
{
	auto* thread = new RunningCaptureThread();
	thread->functor = polling_functor;
	thread->thread = new boost::thread(boost::bind(&VideoPollingBackend::operator(), polling_functor));
	polling_thread.reset(thread);
	return true;
}

void VideoCaptureSession::stopCapture()
{
	// blocks until the next opportunity to stop the capture.
	polling_thread.reset(nullptr);
}

const VideoPollingBackend* VideoCaptureSession::getBackend() const
{
	return polling_functor;
}
