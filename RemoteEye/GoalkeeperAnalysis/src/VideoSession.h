#ifndef VIDEO_SOURCE_H_
#define VIDEO_SOURCE_H_

#include <functional>

class VideoSession
{
public:
	virtual ~VideoSession() = default;
	virtual bool startCapture(std::function<void(unsigned char*, unsigned int)> callback) = 0;
	virtual void stopCapture() = 0;
};

#endif