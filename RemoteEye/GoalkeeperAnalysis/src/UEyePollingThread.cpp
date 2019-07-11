#include "UEyePollingThread.h"

#include "../uEyeCamera.h"

UEyePollingThread::UEyePollingThread(uEyeCamera* camera, VideoCaptureSession::CallbackType callback) :
	camera(camera),
	callback(callback),
	stop_flag(false) { }


void UEyePollingThread::operator()()
{

	camera->InitMemory();
	camera->setFlash();

	if (!camera->camOpened)
	{
		camera->openCamera();
	}
	
	if(!camera->startCapture())
	{
		qWarning() << "Couldn't open camera capture.";
		return;
	}

	char* pBuffer;
	INT nMemID = 0;

	while (!stop_flag)
	{
		if (camera->getNextImage(&pBuffer, &nMemID))
		{
			//TODO: uEyeCamera class cleanup, m_nBitsPerPixel is a global define established by the class, as opposed to a class member
			auto frame = cv::Mat(camera->actualHeight, camera->actualWidth, CV_8UC3, pBuffer);
			//auto frame = cv::Mat(camera->sensorInfo.nMaxHeight, camera->sensorInfo.nMaxWidth, CV_8UC3, pBuffer);

			callback(frame);
			camera->unlockSequenceBuffer(&nMemID, &pBuffer);
		}
	}

	camera->stopVideo();
}

void UEyePollingThread::stop()
{
	stop_flag = true;
}

