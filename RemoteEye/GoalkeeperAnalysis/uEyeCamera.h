#ifndef UEYECAMERA_H_
#define UEYECAMERA_H_

#include "uEye.h"
#include <uEye_tools.h>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QDebug>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <boost\exception\all.hpp>


class uEyeCamera
{
public:

	//PUBLIC VARIABLES
	SENSORINFO sensorInfo;
	IS_RECT rectAOI;
	UINT nPixelClock;
	HIDS currentCam;
	IMAGE_FORMAT_LIST* pformatList; // data structure containing all formats
	INT curColorMode;
	INT curDisplayMode;
	DWORD curMaxWidth;
	DWORD curMaxHeight;
	BOOLEAN curIsAnalogGain;
	BOOL curIsRedGain;
	BOOL curIsGreenGain;
	BOOL curIsBlueGain;
	BOOL curIsGlobalShutter;
	WORD curSizeOfPixelInMicroMeter;
	char curUpperleftBayerPixel;
	CAMINFO cameraInfo; // data structure containing camera information
	WORD curSensorID;
	char curSensorName[32];
	QString QStringColorMode;
	char curCamSerialNum[12];
	char curProducerID[20];
	QString company;
	unsigned char camType;
	boolean camOpened;

	int actualWidth;
	int actualHeight;


	// PUBLIC FUNCTIONS
	uEyeCamera();
	boolean setAOI(int x, int y, int width, int height);
	boolean openCamera();
	~uEyeCamera();


	UINT cameraDeviceID;
	void showError();
	void activateInternalMemory();
	void setColorMode();
	void getSensorInformation();
	void getAOI();
	QString getReturnCode(INT code);
	boolean closeCamera();
	void setFlash();
	void brightenUpCapturing();
	void setFrameRate(int frameRate);
	void setExposure(float time);
	void setPixelclock();
	void getCameraInformation();
	void InitMemory();
	void destroyMemoryPool();
	boolean exit();
	boolean stopVideo();
	void forceQuit();
	void clearMem();
	boolean startCapture();
	boolean getNextImage(char** pBuffer, INT* nMemID);
	void getCurrentFPS(double* fps);
	void unlockSequenceBuffer(INT* nMemID, char** pBuffer);
	void setGain(int newVal);

	void setGamma(int newVal);

	void setGainBoost();

	void setHWGain(int val);

private:

	// PRIVATE VARIABLES/CONSTANTS
#define colorMode IS_CM_BGR8_PACKED		// COLOR MODE BGR(8 8 8), 
#define displayMode IS_SET_DM_DIB		// Display Mode Bild in Systemspeicher(RAM) erfassen. Darstellung erfolgt ueber is_RenderBitmap() (Standardeinstellung).
#define MAX_SEQ_BUFFER	3		//40
#define m_nBitsPerPixel 24
	int sequenceMemoryID[MAX_SEQ_BUFFER]; // size of camera internal Sequence buffer
	char* sequenceMemoryPointer[MAX_SEQ_BUFFER];
	int sequenceNumberID[MAX_SEQ_BUFFER];
	double FPS;
	double newFPS;
	double dblFPS;
	boolean captureFlag;
	size_t numberOfFrames; // Count of memory blocks for setting up camera internal memory space


	// PRIVATE FUNCTIONS

	void printColorMode(int curColorMode);
	void printSensorTypeAsString(WORD curSensorType);
	void printErrorCode(int error);
};

#endif // UEYECAMERA_H
