#ifndef UEYE_POLLING_THREAD_H_
#define UEYE_POLLING_THREAD_H_

#include <atomic>
#include "VideoCaptureSession.h"
class uEyeCamera;

class UEyePollingThread : public VideoPollingBackend
{
public:
	explicit UEyePollingThread(uEyeCamera* camera, VideoCaptureSession::CallbackType callback);
	void operator()();

	void stop() override;

private:
	uEyeCamera* camera;
	VideoCaptureSession::CallbackType callback;
	std::atomic<bool> stop_flag;
};

#endif
