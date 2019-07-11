#include <uEyeCamera.h>


uEyeCamera::uEyeCamera()
{
	// 1. Get all attached cameras
	INT nNumCam;
	if (is_GetNumberOfCameras(&nNumCam) == IS_SUCCESS)
	{
		int iCamera;
		if (nNumCam >= 1)
		{
			// Liste neu mit passender Größe anlegen
			UEYE_CAMERA_LIST* pucl;
			pucl = (UEYE_CAMERA_LIST*)new BYTE[sizeof(ULONG) + nNumCam * sizeof(UEYE_CAMERA_INFO)];
			pucl->dwCount = nNumCam;

			//Kamerainformationen einlesen
			if (is_GetCameraList(pucl) == IS_SUCCESS)
			{
				qDebug() << "Listing all available cameras:";
				for (iCamera = 0; iCamera < (int)pucl->dwCount; iCamera++)
				{
					//Kamerainformationenam am Bildschirm ausgeben
					qDebug() << "Camera: " << iCamera << "  Device-ID: " << pucl->uci[iCamera].dwCameraID;
					cameraDeviceID = pucl->uci[iCamera].dwCameraID;
					currentCam = iCamera;
				}
			}
			delete[] pucl;
		}
	}
	else
	{
		qDebug() << "No uEye Camera attached to system.";
	}


	numberOfFrames = 3;
	// 1. open camera
	if (openCamera())
	{
		qDebug() << "Ready to capture";
	}
}

boolean uEyeCamera::setAOI(int x, int y, int width, int height)
{



	/* Struktur anlegen */
	IS_MULTI_AOI_CONTAINER * m_psMultiAOIs = new IS_MULTI_AOI_CONTAINER;
	m_psMultiAOIs->nNumberOfAOIs = 0;

	/* Abfrage wie viele AOIs gesetzt werden können */
	int m_nMaxNumberMultiAOIs = 0;
	is_AOI(currentCam, IS_AOI_MULTI_GET_AOI | IS_AOI_MULTI_MODE_GET_MAX_NUMBER, (void *)&m_nMaxNumberMultiAOIs, sizeof(m_nMaxNumberMultiAOIs));

	qDebug() << "maximum AOI: " << m_nMaxNumberMultiAOIs;

	/* Abfrage der minimalen Größe einer einzelnen AOI im Multi-AOI-Modus */
	IS_SIZE_2D rMinSizeAOI;
	is_AOI(currentCam, IS_AOI_MULTI_GET_AOI | IS_AOI_MULTI_MODE_GET_MINIMUM_SIZE, (void *)&rMinSizeAOI, sizeof(rMinSizeAOI));

	qDebug() << "minimal size of AOI: " << rMinSizeAOI.s32Width << " x " << rMinSizeAOI.s32Width;

	/* Wenn diese Art Multi-AOI unterstützt wird, alloziere den entsprechenden Speicher */
	if (m_nMaxNumberMultiAOIs > 1)
	{
		/* Hinweis: Struktur für Multi-AOI immer für die maximale Anzahl der einstellbaren AOIs anlegen. */
		m_psMultiAOIs->nNumberOfAOIs = m_nMaxNumberMultiAOIs;
		m_psMultiAOIs->pMultiAOIList = new IS_MULTI_AOI_DESCRIPTOR[m_nMaxNumberMultiAOIs];
		ZeroMemory(m_psMultiAOIs->pMultiAOIList, sizeof(IS_MULTI_AOI_DESCRIPTOR) * m_nMaxNumberMultiAOIs);

		/* AOI setzen an Position (50/50) mit Größe (650/300) */
		m_psMultiAOIs->pMultiAOIList[0].nPosX = x;
		m_psMultiAOIs->pMultiAOIList[0].nPosY = y;
		m_psMultiAOIs->pMultiAOIList[0].nWidth = width;
		m_psMultiAOIs->pMultiAOIList[0].nHeight = height;
		m_psMultiAOIs->pMultiAOIList[0].nStatus = IS_AOI_MULTI_STATUS_SETBYUSER; /* Initial definiert als Benutzerdefinierte AOI */

		/* Verifiziere die Konfiguration */
		int nRet = is_AOI(currentCam, IS_AOI_MULTI_SET_AOI | IS_AOI_MULTI_MODE_ONLY_VERIFY_AOIS, (void*)m_psMultiAOIs, sizeof(IS_MULTI_AOI_CONTAINER) + sizeof(IS_MULTI_AOI_DESCRIPTOR) * m_psMultiAOIs->nNumberOfAOIs);

		qDebug() << "Setting AOI to => x: " << m_psMultiAOIs->pMultiAOIList[0].nPosX << " y: " << m_psMultiAOIs->pMultiAOIList[0].nPosY
			<< "width : " <<  m_psMultiAOIs->pMultiAOIList[0].nWidth << " height: " << m_psMultiAOIs->pMultiAOIList[0].nHeight;


		/* Wenn Konfiguration gültig, setze konfiguration/etc. */
		if (nRet == IS_SUCCESS) 
		{
			qDebug() << " AOI Configuration is valid. Setting config. ";
			nRet = is_AOI(currentCam, IS_AOI_MULTI_SET_AOI, (void*)m_psMultiAOIs, sizeof(IS_MULTI_AOI_CONTAINER) + sizeof(IS_MULTI_AOI_DESCRIPTOR) * m_psMultiAOIs->nNumberOfAOIs);
			this->actualWidth = width;
			this->actualHeight = height;
		}
		else
		{
			this->actualWidth = sensorInfo.nMaxWidth;
			this->actualHeight = sensorInfo.nMaxHeight;
			qDebug() << " AOI Configuration is INvalid. Couldnt set to new AOI. ";
		}

		/* Frage die aktuelle Konfiguration ab */
		is_AOI(currentCam, IS_AOI_MULTI_GET_AOI, (void *)m_psMultiAOIs, m_psMultiAOIs->nNumberOfAOIs * sizeof(IS_MULTI_AOI_DESCRIPTOR) + sizeof(IS_MULTI_AOI_CONTAINER));

		/* Zähle die verschiedenen AOIs */
		int nUsed = 0;
		int nComplemented = 0;
		int nErronous = 0;
		int nConflicted = 0;
		for (int i = 0; i < m_nMaxNumberMultiAOIs; i++) {
			/* Zähle die verwendeten AOIs */
			if (!(m_psMultiAOIs->pMultiAOIList[i].nStatus & IS_AOI_MULTI_STATUS_UNUSED) && (m_psMultiAOIs->pMultiAOIList[i].nWidth > 0) && (m_psMultiAOIs->pMultiAOIList[i].nHeight > 0)) {
				nUsed++;
			}

			/* Zähle die ergänzten AOIs */
			if (m_psMultiAOIs->pMultiAOIList[i].nStatus & IS_AOI_MULTI_STATUS_COMPLEMENT) {
				nComplemented++;
			}

			/* Zähle die fehlerhaften AOIs */
			if (m_psMultiAOIs->pMultiAOIList[i].nStatus & IS_AOI_MULTI_STATUS_ERROR) {
				nErronous++;
			}

			/* Zähle die konfliktbehafteten AOIs (z.b. Überlappung) */
			if (m_psMultiAOIs->pMultiAOIList[i].nStatus & IS_AOI_MULTI_STATUS_CONFLICT) {
				nConflicted++;
			}
		}

	}

	/* Gebe den Speicher wieder frei */
	//TODO: Das hier so gestalten das es auch funktioniert wenn die Kamera nicht erfolgreich 
	// speicher reserviert / u.a. weil die kamera nicht existiert oder angeschlossen ist.
	/*if (m_psMultiAOIs)
	{
		if (m_psMultiAOIs->pMultiAOIList)
		{
			delete[] m_psMultiAOIs->pMultiAOIList;
		}
		delete m_psMultiAOIs;
		m_psMultiAOIs = NULL;
	}*/



	
		return true;
	

	/*
	IS_RECT rectAOI;

	rectAOI.s32X = x;
	rectAOI.s32Y = y;
	rectAOI.s32Width = width;
	rectAOI.s32Height = height;
	INT nRet = is_AOI(currentCam, IS_AOI_IMAGE_SET_AOI, (void*)&rectAOI, sizeof(rectAOI));

	if (nRet == IS_SUCCESS)
	{
		return true;
	}
	else
	{
		return false;
	}
	*/
}


boolean uEyeCamera::openCamera()
{
	INT nRet3 = is_InitCamera(&currentCam, NULL);

	if (nRet3 == IS_SUCCESS)
	{
		qDebug() << "Camera opened";
		camOpened = true;
		return true;
	}
	else
	{
		qDebug() << "Camera NOT OPENED";
		closeCamera();
		return false;
	}
}

void uEyeCamera::activateInternalMemory()
{
	// ACTIVATE INTERNAL CAMERA MEMORY! FASTER.
	// Via camera handle
	int nMemoryMode;
	INT nRet2 = is_DeviceFeature(currentCam, IS_DEVICE_FEATURE_CMD_GET_MEMORY_MODE_ENABLE, &nMemoryMode,
	                             sizeof(nMemoryMode));
	if (nMemoryMode == IS_MEMORY_MODE_ON)
	{
		qDebug() << "Internal Memory: activated";
	}
	else
	{
		// Enable memory mode, device ID must be used for this purpose
		UINT nEnable = IS_MEMORY_MODE_ON;
		INT nRet = is_DeviceFeature(1 | IS_USE_DEVICE_ID, IS_DEVICE_FEATURE_CMD_SET_MEMORY_MODE_ENABLE, (void*)&nEnable,
		                            sizeof(nEnable));
		if (nRet == IS_SUCCESS)
		{
			qDebug() << "Internal Memory: activation successfull.";
		}
		else
		{
			qDebug() << "Couldn't acitvated internal image memory";
		}
	}
}


void uEyeCamera::setColorMode()
{
	//Farbmodus der Kamera setzen
	//INT colorMode = IS_CM_CBYCRY_PACKED;
	INT cMode = IS_CM_BGR8_PACKED;


	INT nRet = is_SetColorMode(currentCam, cMode);
	qDebug() << "colormode set" << endl;
}

void uEyeCamera::getSensorInformation()
{
	INT nRet = is_GetSensorInfo(currentCam, &sensorInfo);
	qDebug() << "\n\n==============================================";
	qDebug() << "Get SensorInformation : " << getReturnCode(nRet);

	//Gibt den Sensortyp zurueck(z.B.: IS_SENSOR_UI224X_C)
	WORD curSensorID = sensorInfo.SensorID;
	// Get sensor type

	if (curSensorID != NULL)
	{
		qDebug() << "Sensor Type: " << curSensorID;
	}
	else
	{
		qDebug() << " Sensor Type: NULL";
	}

	// Gibt das Kameramodell zurueck(z.B.: UI224xLE - C)
	for (int i = 0; i < 12; i++)
	{
		curSensorName[i] = sensorInfo.strSensorName[i];
	}
	// Camera model
	qDebug() << "Camera model: " << QString::fromLocal8Bit(curSensorName);


	//Gibt den Sensor - Farbmodus zurueck
	curColorMode = sensorInfo.nColorMode;
	printColorMode(curColorMode);

	// Gibt die maximale Bildbreite zurueck
	curMaxWidth = sensorInfo.nMaxWidth;

	// Gibt die maximale Bildhöhe zurueck
	curMaxHeight = sensorInfo.nMaxHeight;

	//Gibt an, ob der Sensor eine analoge Gesamtverstärkung bietet
	curIsAnalogGain = sensorInfo.bMasterGain;

	//Gibt an, ob der Sensor eine analoge Rot - Verstärkung bietet
	curIsRedGain = sensorInfo.bRGain;


	//Gibt an, ob der Sensor eine analoge Gruen - Verstärkung bietet
	curIsGreenGain = sensorInfo.bGGain;


	//Gibt an, ob der Sensor eine analoge Blau - Verstärkung bietet
	curIsBlueGain = sensorInfo.bBGain;


	//Gibt an, ob der Sensor einen Global - Shutter besitzt
	// TRUE = GlobalShutter
	// FALSE = RollingShutter
	curIsGlobalShutter = sensorInfo.bGlobShutter;


	//Gibt die Größe der Pixel in μm(z.B. 465 entspricht 4, 65 μm)
	curSizeOfPixelInMicroMeter = sensorInfo.wPixelSize;

	// Gibt die Farbe des ersten Pixels(oben links) an:
	// 0 = BAYER_PIXEL_RED
	// 1 = BAYER_PIXEL_GREEN
	// 2 = BAYER_PIXEL_BLUE
	curUpperleftBayerPixel = sensorInfo.nUpperLeftBayerPixel;


	// max Image width
	qDebug() << "Max. Image Width: " << QString::number(curMaxWidth);

	// Max Image Height
	qDebug() << "Max. Image Height: " << QString::number(curMaxHeight);


	// Does sensor have Analog Master Gain?
	if (curIsAnalogGain)
	{
		qDebug() << "Analog Gain: yes";
	}
	else
	{
		qDebug() << "Analog Gain: no";
	}

	// Does sensor have Analog Red Gain?
	if (curIsRedGain)
	{
		qDebug() << "Analog Red Gain: yes";
	}
	else
	{
		qDebug() << "Analog Red Gain: no";
	}


	// Does sensor have Analog Green gain?
	if (curIsGreenGain)
	{
		qDebug() << "Analog Green Gain: yes";
	}
	else
	{
		qDebug() << "Analog Green Gain: no";
	}

	// Does sensor have analog blue gain?

	if (curIsBlueGain)
	{
		qDebug() << "Analog Blue Gain: yes";
	}
	else
	{
		qDebug() << "Analog Blue Gain: no";
	}


	// Does sensor have global shutter?
	if (curIsGreenGain)
	{
		qDebug() << "Global Shutter: yes";
	}
	else
	{
		qDebug() << "Global Shutter: no";
	}

	//Get size of pixel in μm(z.B. 465 means 4, 65 μm)
	qDebug() << "Size of Pixel in micrometer" << QString::number(curSizeOfPixelInMicroMeter);


	// get color of bayer pixel up left
	switch (curUpperleftBayerPixel)
	{
	case 0:
		qDebug() << "Color of First Pixel up left(Bayer Pixel): red";
		break;

	case 1:
		qDebug() << "Color of First Pixel up left(Bayer Pixel): green";
		break;

	case 2:
		qDebug() << "Color of First Pixel up left(Bayer Pixel): blue";
		break;
	default:
		qDebug() << "Color of First Pixel up left(Bayer Pixel): n.a.";
		break;
	}
}

void uEyeCamera::getAOI()
{
	INT nRet = is_AOI(currentCam, IS_AOI_IMAGE_GET_AOI, (void*)&rectAOI, sizeof(rectAOI));
	if (nRet == IS_SUCCESS)
	{
		qDebug() << "Getting  AOI was successfull";
	}
	else
	{
		qDebug() << "ERROR:: Cant get aoi";
	}
}

void uEyeCamera::getCameraInformation()
{
	INT nRet = is_GetCameraInfo(currentCam, &cameraInfo);
	qDebug() << "\n\n==============================================";
	qDebug() << "Get CameraInformation: " << getReturnCode(nRet);

	for (int i = 0; i < 12; i++)
	{
		curCamSerialNum[i] = cameraInfo.SerNo[i];
	}

	for (int i = 0; i < 8; i++)
	{
		curProducerID[i] = cameraInfo.ID[i];
		company += curProducerID[i];
	}

	//Kameratyp
	camType = cameraInfo.Type;


	// Serial number of camera
	qDebug() << "SerNo: " << QString::fromUtf8(curCamSerialNum);

	// Company
	qDebug() << "Company: " << company;

	switch (camType)
	{
	case IS_CAMERA_TYPE_UEYE_USB_SE:
		qDebug() << "Camera Type: USB uEye SE";
		break;

	case IS_CAMERA_TYPE_UEYE_USB_LE:
		qDebug() << "Camera Type: USB uEye LE";
		break;

	case IS_CAMERA_TYPE_UEYE_USB_ML:
		qDebug() << "Camera Type: USB uEye ML";
		break;

	case IS_CAMERA_TYPE_UEYE_USB3_CP:
		qDebug() << "Camera Type: USB 3 uEye CP";
		break;

	case IS_CAMERA_TYPE_UEYE_USB3_LE:
		qDebug() << "Camera Type: USB 3 uEye LE";
		break;

	case IS_CAMERA_TYPE_UEYE_USB3_ML:
		qDebug() << "Camera Type: USB 3 uEye ML";
		break;

	case IS_CAMERA_TYPE_UEYE_USB3_XC:
		qDebug() << "Camera Type: USB 3 uEye XC";
		break;

	case IS_CAMERA_TYPE_UEYE_ETH_SE:
		qDebug() << "Camera Type: GigE uEye SE";
		break;

	case IS_CAMERA_TYPE_UEYE_ETH_REP:
		qDebug() << "Camera Type: GigE uEye RE PoE";
		break;

	case IS_CAMERA_TYPE_UEYE_ETH_CP:
		qDebug() << "Camera Type:GigE uEye CP";
		break;

	case IS_CAMERA_TYPE_UEYE_ETH_LE:
		qDebug() << "Camera Type: GigE uEye LE";
		break;

	case IS_CAMERA_TYPE_UEYE_PMC:
		qDebug() << "Camera Type: Virtuel Multicast - camera";
		break;
	default:
		qDebug() << "Camera Type: n.a.";
		break;
	}
}

QString uEyeCamera::getReturnCode(INT code)
{
	QString name;

	if (code == IS_SUCCESS)
	{
		name = "success";
	}
	else
	{
		name = QString::number(code);
	}

	//return name;

	switch (code)
	{
	case IS_AVI_NO_ERR:
		name = " success";
		break;
	case IS_AVI_ERR_PARAMETER:
		name = "Einer der uebergebenen Parameter ist außerhalb des gueltigen Bereichs.";
		break;

	case IS_AVI_ERR_NO_CODEC_AVAIL:
		name =
			"Die maximale Anzahl auf diesem System möglicher Instanzen wurde erreicht.Es kann keine weitere Instanz erzeugt werden.";

		break;
	case IS_AVI_ERR_INVALID_UEYE:
		name = "Es wurde keine uEye Kamera gefunden.";
		break;

	case IS_ALL_DEVICES_BUSY:
		name = "All cameras in use";
		break;

	case IS_BAD_STRUCTURE_SIZE:
		name = "An internal structure got a wrong size";
		break;

	case IS_CANT_ADD_TO_SEQUENCE:
		name = "Der Bildspeicher befindet sich bereits in der Sequenz und kann nicht doppelt hinzugefuegt werden.";
		break;

	case IS_CANT_COMMUNICATE_WITH_DRIVER:
		name = " Kommunikation mit dem Kameratreiber schlug fehl, weil kein Treiber geladen ist.";
		break;

	case IS_CANT_OPEN_DEVICE:
		name = "Ein Versuch die Kamera zu öffnen schlug fehl(Kamera nicht vorhanden oder Fehler beim Initialisieren).";
		break;
	case IS_CANT_OPEN_REGISTRY:
		name = "Fehler beim Öffnen eines Windows Registry Keys";
		break;

	case IS_CANT_READ_REGISTRY:
		name = "Fehler beim Lesen von Einstellungen aus der Windows Registry";
		break;

	case IS_CAPTURE_RUNNING:
		name = " Es läuft bereits eine Aufnahme, die zuerst beendet werden muss.";
		break;

	case IS_CRC_ERROR:
		name = "Beim Lesen der Einstellungen trat ein Problem mit der CRC - Fehlerkorrektur auf.";
		break;
	case IS_DEVICE_ALREADY_PAIRED:
		name = "Das Gerät ist bereits angeschlossen.";
		break;

	case IS_DEVICE_NOT_COMPATIBLE:
		name = "Die Kamera ist nicht kompatibel zu den Treibern.";
		break;

	case IS_DR_CANNOT_CREATE_SURFACE:
		name = " Bild - oder Overlay - Surface konnten nicht angelegt werden.";
		break;

	case IS_DR_CANNOT_CREATE_TEXTURE:
		name = "Die Textur konnte nicht angelegt werden.";
		break;

	case IS_DR_CANNOT_CREATE_VERTEX_BUFFER:
		name = "Der Vertex - Puffer konnte nicht angelegt werden.";
		break;

	case IS_DR_DEVICE_OUT_OF_MEMORY:
		name = "Nicht genuegend Grafikspeicher verfuegbar.";
		break;

	case IS_DR_LIBRARY_NOT_FOUND:
		name = "Die DirectRenderer - Bibliothek konnte nicht gefunden werden.";
		break;

	case IS_ERROR_CPU_IDLE_STATES_CONFIGURATION:
		name = "Die Konfiguration des CPU - Ruhestands ist fehlgeschlagen.";
		break;

	case IS_FILE_WRITE_OPEN_ERROR:
		name = "Datei kann nicht zum Schreiben oder Lesen geöffnet werden.";
		break;

	case IS_INCOMPATIBLE_SETTING:
		name = "Die Funktion ist nicht möglich, da andere Einstellungen nicht kompatibel sind.";
		break;

	case IS_INVALID_BUFFER_SIZE:
		name = "Der Bildspeicher hat die falsche Größe um das Bild im gewuenschten Format aufzunehmen.";
		break;


	case IS_INVALID_CAMERA_TYPE:
		name = "Kameratyp der.ini - Datei stimmt nicht mit dem aktuellen Kameramodell ueberein.";
		break;

	case IS_INVALID_CAPTURE_MODE:
		name =
			"Die Funktion kann im aktuellen Betriebsmodus der Kamera(Freilaufend, Trigger oder Standby) nicht ausgefuehrt werden.";
		break;

	case IS_INVALID_DEVICE_ID:
		name =
			"Die Device - ID ist ungueltig.Gueltige IDs beginnen bei 1 fuer USB - Kameras und bei 1001 fuer Gigabit - Ethernet - Kameras.";
		break;

	case IS_INVALID_EXPOSURE_TIME:
		name = "Diese Einstellung ist bei der aktuell eingestellten Belichtungszeit nicht möglich.";
		break;

	case IS_INVALID_CAMERA_HANDLE:
		name = "Ungueltiges Kamera - Handle";
		break;

	case IS_INVALID_IP_CONFIGURATION:
		name = "Die Konfiguration der IP - Adresse ist ungueltig.";
		break;

	case IS_INVALID_MEMORY_POINTER:
		name = "Ungueltiger Zeiger oder ungueltige Speicher - ID";
		break;

	case IS_INVALID_MODE:
		name = "	Kamera im Standby, Funktion nicht erlaubt";
		break;

	case IS_INVALID_PARAMETER:
		name =
			"Einer der uebergebenen Parameter ist außerhalb des gueltigen Bereichs, oder fuer diesen Sensor nicht unterstuetzt, bzw.in diesem Modus nicht zugänglich.";
		break;

	case IS_INVALID_PIXEL_CLOCK:
		name = "Diese Einstellung ist bei dem aktuell eingestellten Pixeltakt nicht möglich.";
		break;

	case IS_IO_REQUEST_FAILED:
		name =
			"Eine IO Anforderung des uEye Treibers schlug fehl.Eventuell passen die Versionen der Dateien ueye_api.dll(API) und ueye_usb.sys bzw.ueye_eth.sys(Treiber) nicht zusammen.";
		break;

	case IS_NETWORK_CONFIGURATION_INVALID:
		name = "Die Konfiguration der Netzwerkkarte ist ungueltig.";
		break;

	case IS_NETWORK_FRAME_SIZE_INCOMPATIBLE:
		name = "Die Bildgrößen - Einstellungen der Kamera sind nicht kompatibel zur PC - Netzwerkkarte.";
		break;

	case IS_NO_ACTIVE_IMG_MEM:
		name =
			"Kein aktivierter Bildspeicher vorhanden.Der Speicher muss mit der Funktion is_SetImageMem() aktiviert werden, oder es muss mit der Funktion is_AddToSequence() eine Sequenz aufgebaut werden.";
		break;

	case IS_NO_IMAGE_MEM_ALLOCATED:
		name = "Der Treiber konnte keinen Speicher reservieren.";
		break;

	case IS_NO_IR_FILTER:
		name = "Kein IR - Filter vorhanden";
		break;

	case IS_NO_SUCCESS:
		name = "Allgemeine Fehlermeldung";
		break;

	case IS_NOT_CALIBRATED:
		name = "Die Kamera enthält keine Kalibrierungsdaten.";
		break;

	case IS_NOT_SUPPORTED:
		name = "Das verwendete Kameramodell unterstuetzt diese Funktion oder Einstellung nicht.";
		break;

	case IS_NULL_POINTER:
		name = "Ungueltiges Array";
		break;

	case IS_OUT_OF_MEMORY:
		name = "Es konnte kein Speicher alloziert werden.";
		break;


	case IS_SEQUENCE_BUF_ALREADY_LOCKED:
		name = "Der Speicher konnte nicht gelockt werden.Der Zeiger auf den Buffer ist ungueltig.";
		break;

	case IS_STARTER_FW_UPLOAD_NEEDED:
		name = "Die Starter Firmware der Kamera ist mit dem Treiber inkompatibel und muss aktualisiert werden.";
		break;

	case IS_SUBNET_MISMATCH:
		name = "Das Subnet der Kamera und der PC - Netzwerkkarte sind unterschiedlich.";
		break;

	case IS_SUBNETMASK_MISMATCH:
		name = "Die Subnet - Maske der Kamera und der PC - Netzwekkarte sind unterschiedlich.";
		break;

	case IS_TIMED_OUT:
		name = "Ein Timeout trat auf.Eine Bildaufnahme konnte nicht in der vorgeschriebenen Zeit beendet werden.";
		break;

	case IS_TRIGGER_ACTIVATED:
		name = "Die Funktion ist nicht möglich, da die Kamera auf ein Triggersignal wartet.";
		break;

	case IS_GET_DISPLAY_MODE:
		name = "	Aktuelle Einstellung";
		break;


	case IS_INVALID_COLOR_FORMAT:
		name = "Ungueltiges Farbformat.";
		break;


	case DR_CHECK_COMPATIBILITY:
		name = "DR_CHECK_COMPATIBILITY";
		break;


	case IS_DR_DEVICE_CAPS_INSUFFICIENT:
		name = "Die Grafikhardware bietet keine vollständige Unterstuetzung fuer die uEye Direct3D-Funktionen";
		break;


	case IS_DR_CANNOT_GET_OVERLAY_DC:
		name = "Das Device-Context-Handle fuer das Overlay konnte nicht geholt werden.";
		break;


	case IS_DR_CANNOT_LOCK_OVERLAY_SURFACE:
		name = "Das Overlay-Surface konnte nicht gesperrt werden.";
		break;


	case IS_DR_CANNOT_RELEASE_OVERLAY_DC:
		name = "Das Device-Context-Handle fuer das Overlay konnte nicht freigegeben werden.";
		break;


	case IS_DR_CANNOT_UNLOCK_OVERLAY_SURFACE:
		name = "Das Overlay-Surface konnte nicht freigegeben werden.";
		break;

	case IS_DR_NOT_ALLOWED_WHILE_DC_IS_ACTIVE:
		name = "Die Anwendung hat noch ein Device-Context-Handle offen.";
		break;

	default:
		name = QString::number(code);
		break;
	}
	return name;
}

boolean uEyeCamera::closeCamera()
{
	INT returnCloseCamera = is_ExitCamera(currentCam);
	if (returnCloseCamera == IS_SUCCESS)
	{
		camOpened = false;
		return true;
	}
	else
	{
		camOpened = true;
		return false;
	}
}

void uEyeCamera::setFlash()
{
	qDebug() << "\n\n==============================================";
	qDebug() << "Set FLASH: ";


	//SET GPIO as Flash Output

	UINT nMode = IO_FLASH_MODE_FREERUN_HI_ACTIVE;
	//UINT nMode = IO_FLASH_MODE_CONSTANT_HIGH;
	INT nRet = is_IO(currentCam, IS_IO_CMD_FLASH_SET_MODE, (void*)&nMode, sizeof(nMode));

	// Get the current flash mode
	nRet = is_IO(currentCam, IS_IO_CMD_FLASH_GET_MODE, (void*)&nMode, sizeof(nMode));

	switch (nMode)
	{
	case IO_FLASH_MODE_OFF:
		qDebug() << "Flash: Flashoutput deactivated.";
		break;

	case IO_FLASH_MODE_TRIGGER_LO_ACTIVE:
		qDebug() << "Flash:  Activated Flash in trigger mode. Digital input will be set to low during flash.";
		break;

	case IO_FLASH_MODE_TRIGGER_HI_ACTIVE:
		qDebug() << "Flash:  Activated Flash in trigger mode. Digital input will be set to high during flash.";
		break;

	case IO_FLASH_MODE_CONSTANT_HIGH:
		qDebug() << "Flash:  Digital output set to constant high.";
		break;

	case IO_FLASH_MODE_CONSTANT_LOW:
		qDebug() << "Flash:   Digital output set to constant low.";
		break;

	case IO_FLASH_MODE_FREERUN_LO_ACTIVE:
		qDebug() << "Flash: Activated falsh in free run mode. Digital output ist set to low during flash.";
		break;

	case IO_FLASH_MODE_FREERUN_HI_ACTIVE:
		qDebug() << "Flash: Activated falsh in free run mode. Digital output ist set to high during flash.";
		break;
	default:
		qDebug() << "Flash: WTF. Just nope.";
		break;
	}

	IO_FLASH_PARAMS flashParams;
	nRet = is_IO(currentCam, IS_IO_CMD_FLASH_SET_GPIO_PARAMS, (void*)&flashParams, sizeof(flashParams));

	// GPIO 1 configuration
	IO_GPIO_CONFIGURATION gpioConfiguration;

	gpioConfiguration.u32Gpio = IO_GPIO_1;
	gpioConfiguration.u32Configuration = IS_GPIO_FLASH;


	if (IS_SUCCESS == is_IO(currentCam, IS_IO_CMD_GPIOS_SET_CONFIGURATION, (void*)&gpioConfiguration,
	                        sizeof(gpioConfiguration)))
	{
		qDebug() << "GPIO1 activated as flash output";
	}
	else
	{
		qDebug() << "GPIO1 NOT activated as flash output";
	}

	//INT ret = is_SetGlobalShutter(*currentCam, IS_SET_GLOBAL_SHUTTER_ON);


	// Get the minimum values for the GPIO flash delay and flash duration
	nRet = is_IO(currentCam, IS_IO_CMD_FLASH_GET_GPIO_PARAMS_MIN, (void*)&flashParams, sizeof(flashParams));
	if (IS_SUCCESS == nRet)
	{
		qDebug() << "Get min Flash delay: " << QString::number(flashParams.s32Delay);
		qDebug() << "Get min Flash Duration: " << QString::number(flashParams.u32Duration);
	}
	else
	{
		qDebug() << "Could not get flash delay and duration";
		qDebug() << "Could not get flash delay and duration";
	}

	//flashParams.s32Delay = 0;
	//flashParams.u32Duration = 0;
	// Set the minimum values for flash delay and flash duration. Be careful: The normal flash does not work with values < 20 us
	nRet = is_IO(currentCam, IS_IO_CMD_FLASH_SET_GPIO_PARAMS, (void*)&flashParams, sizeof(flashParams));
	if (IS_SUCCESS == nRet)
	{
		qDebug() << "flash delay set " << QString::number(flashParams.s32Delay);
		qDebug() << "flash duration set " << QString::number(flashParams.u32Duration);
	}
	else
	{
		qDebug() << "Could not set flash delay and duration";
		qDebug() << "Could not set flash delay and duration";
	}
}

void uEyeCamera::brightenUpCapturing()
{
	qDebug() << "\n\n==============================================";
	qDebug() << "Brightening Up Capture: ";

	/* Functions to get a brighter image*/
	double pval1;
	double pval2;

	pval1 = 1;
	//Aktiviert / deaktiviert die Auto - Verstärkungs - Funktion
	INT ret = is_SetAutoParameter(currentCam, IS_SET_ENABLE_AUTO_GAIN, &pval1, &pval2);
	if (IS_SUCCESS == ret)
	{
		qDebug() << "Activated Auto Gain sensor";
	}
	else
	{
		qDebug() << "DEactivated Auto Gain sensor";
	}


	//Aktiviert / deaktiviert die interne Auto - Verstärkungs - Funktion bzw.bei HDR - Sensoren die Weißwert - Anpassung des Sensors*1.
	pval1 = 1;
	ret = is_SetAutoParameter(currentCam, IS_SET_ENABLE_AUTO_SENSOR_GAIN, &pval1, &pval2);
	if (IS_SUCCESS == ret)
	{
		qDebug() << "Activated internal Auto Gain sensor";
	}
	else
	{
		qDebug() << "DEactivated internal Auto Gain sensor";
	}

	//Aktiviert / deaktiviert die interne Auto - Belichtungszeit - Funktion des Sensors*1.	
	pval1 = 1;
	ret = is_SetAutoParameter(currentCam, IS_SET_ENABLE_AUTO_SENSOR_SHUTTER, &pval1, &pval2);
	if (IS_SUCCESS == ret)
	{
		qDebug() << "Activated internal exposure time sensor";
	}
	else
	{
		qDebug() << "DEactivated internal exposure time sensor";
	}


	// 	Aktiviert/deaktiviert die Auto-Weißabgleich-Funktion.
	pval1 = 1;
	ret = is_SetAutoParameter(currentCam, IS_SET_ENABLE_AUTO_WHITEBALANCE, &pval1, &pval2);
	if (IS_SUCCESS == ret)
	{
		qDebug() << "Activated auto white balance";
	}
	else
	{
		qDebug() << "DEactivated auto white balance";
	}


	// 	Aktiviert / deaktiviert die Auto - Belichtungszeit - Funktion.
	pval1 = 1;
	ret = is_SetAutoParameter(currentCam, IS_SET_ENABLE_AUTO_SHUTTER, &pval1, &pval2);
	if (IS_SUCCESS == ret)
	{
		qDebug() << "Activated auto exposure time";
	}
	else
	{
		qDebug() << "DEactivated auto exposure time";
	}


	//Set sollwert fürAuto verstärkung und auto-belichtungszeit
	pval1 = 255;

	ret = is_SetAutoParameter(currentCam, IS_SET_AUTO_REFERENCE, &pval1, &pval2);
	if (IS_SUCCESS == ret)
	{
		qDebug() << "Auto Refernce set";
	}
	else
	{
		qDebug() << "Auto Refernce NOT set";
	}


	// SET obere Regelgrenze für Auto Verstärkung
	pval1 = 100;
	ret = is_SetAutoParameter(currentCam, IS_SET_AUTO_GAIN_MAX, &pval1, &pval2);
	if (IS_SUCCESS == ret)
	{
		qDebug() << "Upper Limit for auto gain set";
	}
	else
	{
		qDebug() << "Upper Limit for auto gain NOT set";
	}

	// SET obere Regelgrenze für Auto-Belichtungszeit.
	pval1 = 0;
	ret = is_SetAutoParameter(currentCam, IS_SET_AUTO_SHUTTER_MAX, &pval1, &pval2);
	if (IS_SUCCESS == ret)
	{
		qDebug() << "Upper Limit exposure time set";
	}
	else
	{
		qDebug() << "Upper Limit exposure time NOT set";
	}


	//INT ret = is_SetHWGainFactor(currenctCam, IS_SET_MASTER_GAIN_FACTOR, 357);
}

void uEyeCamera::setFrameRate(int frameRate)
{
	qDebug() << "\n\n==============================================";
	qDebug() << "Setting new Framerate ";

	INT nRet = is_SetFrameRate(currentCam, frameRate, &newFPS);
	if (nRet == IS_SUCCESS)
	{
		std::string varAsString = std::to_string(newFPS);
		qDebug() << "FPS: " << QString::fromStdString(varAsString);
	}
	else
	{
		qDebug() << "FPS not detectable";
	}
}

void uEyeCamera::setExposure(float time)
{
	// Activate Auto exposure
	// Auto-Belichtungszeit deaktivieren
	//Auto-Verstärkung aktivieren:
	double dEnable = 0;
	int ret = is_SetAutoParameter(currentCam, IS_SET_ENABLE_AUTO_GAIN, &dEnable, 0);

	qDebug() << "\n\n==============================================";
	qDebug() << "Setting new Exposure.";
	// Get exposure timing
	UINT nCaps = 0.1;
	double nParam = 0.0;
	INT nRet = is_Exposure(currentCam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void*)&nParam, sizeof(nParam));
	qDebug() << "Current Exposure: " << nParam;

	nRet = is_Exposure(currentCam, IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE_MIN, (void*)&nParam, sizeof(nParam));
	qDebug() << "Min Exposure: " << nParam;

	// 12.0 for 200 FPS
	nParam = time;
	nRet = is_Exposure(currentCam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void*)&nParam, sizeof(nParam));
	qDebug() << "New Exposure: " << nParam;
}

void uEyeCamera::setPixelclock()
{
	// activate extended pixelclock
	UINT nEnable;
	qDebug() << "\n\n==============================================";
	qDebug() << "Setting new Pixelclock";

	/* Get default value for the extended pixel clock range */
	INT nRet = is_DeviceFeature(currentCam, IS_DEVICE_FEATURE_CMD_GET_EXTENDED_PIXELCLOCK_RANGE_ENABLE_DEFAULT,
	                            (void*)&nEnable, sizeof(nEnable));
	if (nRet == IS_SUCCESS)
	{
		/* Set default value */
		nRet = is_DeviceFeature(currentCam, IS_DEVICE_FEATURE_CMD_SET_EXTENDED_PIXELCLOCK_RANGE_ENABLE, (void*)&nEnable,
		                        sizeof(nEnable));
	}
	nRet = is_DeviceFeature(currentCam, IS_DEVICE_FEATURE_CMD_GET_EXTENDED_PIXELCLOCK_RANGE_ENABLE, (void*)&nEnable,
	                        sizeof(nEnable));
	if (nRet == IS_SUCCESS)
	{
		if (nEnable == EXTENDED_PIXELCLOCK_RANGE_ON)
		{
			qDebug() << "Enabling extended Pixelclock";
		}
	}

	/* Enable the extended pixel clock range */
	nEnable = EXTENDED_PIXELCLOCK_RANGE_ON;
	nRet = is_DeviceFeature(currentCam, IS_DEVICE_FEATURE_CMD_SET_EXTENDED_PIXELCLOCK_RANGE_ENABLE, (void*)&nEnable,
	                        sizeof(nEnable));
	if (nRet == IS_SUCCESS)
	{
		qDebug() << "extended Pixelclock IS enabled";
	}


	/* Pixelclock  1. Get count of all possible pixelclock rates*/
	UINT nNumberOfSupportedPixelClocks = 0;
	UINT nPixelClockList[150];

	nRet = is_PixelClock(currentCam, IS_PIXELCLOCK_CMD_GET_NUMBER, (void*)&nNumberOfSupportedPixelClocks,
	                     sizeof(nNumberOfSupportedPixelClocks));
	if ((nRet == IS_SUCCESS) && (nNumberOfSupportedPixelClocks > 0))
	{
		ZeroMemory(&nPixelClockList, sizeof(nPixelClockList));
		nRet = is_PixelClock(currentCam, IS_PIXELCLOCK_CMD_GET_LIST, (void*)nPixelClockList,
		                     nNumberOfSupportedPixelClocks * sizeof(UINT));
	}

	// Pixelclock  2. Get current Pixelclock
	// Get current pixel clock
	nRet = is_PixelClock(currentCam, IS_PIXELCLOCK_CMD_GET, (void*)&nPixelClock, sizeof(nPixelClock));
	qDebug() << "Current Pixelclock: " << QString::number(nPixelClock);


	// Get pixel clock range
	UINT nRange[3];
	ZeroMemory(nRange, sizeof(nRange));
	nRet = is_PixelClock(currentCam, IS_PIXELCLOCK_CMD_GET_RANGE, (void*)nRange, sizeof(nRange));
	if (nRet == IS_SUCCESS)
	{
		UINT nMin = nRange[0];
		UINT nMax = nRange[1];
		UINT nInc = nRange[2];
	}

	// Pixelclock  3. Set pixel clock
	UINT nPixelClockDefault;


	// maximum value leads to bright horizontal and vertical line 
	// choose smaller value
	//nPixelClock = nRange[1];
	nPixelClock = 296;


	// Get default pixel clock
	nRet = is_PixelClock(currentCam, IS_PIXELCLOCK_CMD_GET_DEFAULT, (void*)&nPixelClockDefault,
	                     sizeof(nPixelClockDefault));

	if (nRet == IS_SUCCESS)
	{
		// Set this pixel clock
		nRet = is_PixelClock(currentCam, IS_PIXELCLOCK_CMD_SET, (void*)&nPixelClock, sizeof(nPixelClock));

		// Get current pixel clock
		nRet = is_PixelClock(currentCam, IS_PIXELCLOCK_CMD_GET, (void*)&nPixelClock, sizeof(nPixelClock));
		qDebug() << "Set New Pixelclock: " << QString::number(nPixelClock);
	}
}

void uEyeCamera::InitMemory()
{
	qDebug() << "\n\n==============================================";
	qDebug() << "Initializing memory pool.";

	// check if memory is already nitialized
	// Allocate Image Memories and add them to sequence
	for (size_t i = 0; i < numberOfFrames; ++i)
	{
		// Format changed
		//qDebug() << "AllocateImageMem:" << getReturnCode(is_AllocImageMem(currentCam, sensorInfo.nMaxWidth,
		//                                                                  sensorInfo.nMaxHeight, m_nBitsPerPixel,
		//                                                                  &sequenceMemoryPointer[i], &sequenceMemoryID[i]));

		qDebug() << "AllocateImageMem:" << getReturnCode(is_AllocImageMem(currentCam, this->actualWidth,
			                                                                  this->actualHeight, m_nBitsPerPixel,
			                                                                  &sequenceMemoryPointer[i], &sequenceMemoryID[i]));

		qDebug() << "AddToSequence:" << getReturnCode(is_AddToSequence(currentCam, sequenceMemoryPointer[i], sequenceMemoryID[i]));
		//qDebug() << "AllocateImageMem:" << getReturnCode(is_AllocImageMem(currentCam, wi, he, m_nBitsPerPixel, &sequenceMemoryPointer[i], &sequenceMemoryID[i]));
		//qDebug() << "AddToSequence:" <<  getReturnCode(is_AddToSequence(currentCam, sequenceMemoryPointer[i], sequenceMemoryID[i]));
	}

	// Activate Image Queue
	qDebug() << "InitImageQueue:" << getReturnCode(is_InitImageQueue(currentCam, 0));
}

// destroyMemoryPool()
// destroys camera internal memory pool to release memory space
// not used anymore after capturing
void uEyeCamera::destroyMemoryPool()
{
	qDebug() << "\n\n==============================================";
	qDebug() << "Destroying memory pool.";
	for (int i = 0; i < numberOfFrames; i++)
	{
		is_FreeImageMem(currentCam, sequenceMemoryPointer[i], sequenceMemoryID[i]);
	}
}

uEyeCamera::~uEyeCamera() {}

void uEyeCamera::showError()
{
	UEYE_CAPTURE_STATUS_INFO CaptureStatusInfo;
	INT nRet = is_CaptureStatus(currentCam, IS_CAPTURE_STATUS_INFO_CMD_GET, (void*)&CaptureStatusInfo,
	                            sizeof(CaptureStatusInfo));

	if (nRet == IS_SUCCESS)
	{
		qDebug() << "CONVERSION FAILED: " << QString::number(
			CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_CONVERSION_FAILED]);
		qDebug() << "NO DEST MEM " << QString::
			number(CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_NO_DEST_MEM]);
		qDebug() << "IMAGE LOCKED: " << QString::number(
			CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_IMAGE_LOCKED]);
		qDebug() << "DEVICE NTO READY: " << QString::number(
			CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_DRV_DEVICE_NOT_READY]);
		qDebug() << "OUT OF BUFFERS: " << QString::number(
			CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_DRV_OUT_OF_BUFFERS]);
		qDebug() << "TRANSFER FAILED: " << QString::number(
			CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_USB_TRANSFER_FAILED]);
		qDebug() << "DEvice Time out: " << QString::number(
			CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_DEV_TIMEOUT]);
		qDebug() << "MISSED IMAGE: " << QString::number(
			CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_DEV_MISSED_IMAGES]);
		qDebug() << "BUFFER OVERRUN: " << QString::number(
			CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_ETH_BUFFER_OVERRUN]);
		qDebug() << "CAPUTER FAILURE: " << QString::number(
			CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_DEV_FRAME_CAPTURE_FAILED]);

		qDebug() << "TOTAL ERRORS:" << QString::number(CaptureStatusInfo.dwCapStatusCnt_Total);
	}
}

boolean uEyeCamera::exit()
{
	INT retEx = is_ExitImageQueue(currentCam);
	//iNT retClose = this->closeCamera();
	INT retClose = true;

	if (retEx == IS_SUCCESS && retClose == IS_SUCCESS)
	{
		return true;
	}
	else
	{
		return false;
	}
}

boolean uEyeCamera::stopVideo()
{
	qDebug() << "\n==============================================";
	qDebug() << "Stop Video.";
	INT nRet = is_CaptureVideo(currentCam, IS_GET_LIVE);
	if (nRet)
	{
		if (is_StopLiveVideo(currentCam, IS_WAIT) != IS_SUCCESS)
		{
			qDebug() << "Stopping FAILED";
			return false;
		}
		else
		{
			qDebug() << "Stopping SUCCEDED";
			return true;
		}
	}
	else
	{
		// no capture function currently runnning
		return true;
	}
}

void uEyeCamera::forceQuit()
{
	qDebug() << "\n==============================================";
	qDebug() << "Forcing stop Video.";


	if (is_StopLiveVideo(currentCam,IS_FORCE_VIDEO_STOP) != IS_SUCCESS)
	{
		qDebug() << "Forcing FAILED";
	}
	else
	{
		qDebug() << "Forcing SUCCEDED";
	}
}

void uEyeCamera::clearMem()
{
	qDebug() << "Clearing memory sequence.";
	if (is_ClearSequence(currentCam) != IS_SUCCESS)
	{
		qDebug() << "Could not clear memory.";
	}
	else
	{
		qDebug() << "Clearing Memory was successfull.";
	}
}

boolean uEyeCamera::startCapture()
{
	qDebug() << "\n\n==============================================";
	qDebug() << "Starting capture.";
	INT nRet = is_CaptureVideo(currentCam, IS_WAIT);
	if (nRet != IS_SUCCESS)
	{
		qDebug() << "Start Capture FAILED.";
		qDebug() << getReturnCode(nRet);
		return false;
	}
	else
	{
		qDebug() << "Start capture SUCCEDED.";
		return true;
	}
}

boolean uEyeCamera::getNextImage(char** pBuffer, INT* nMemID)
{
	if (IS_SUCCESS == is_WaitForNextImage(currentCam, 1000, pBuffer, nMemID))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void uEyeCamera::getCurrentFPS(double* fps)
{
	is_GetFramesPerSecond(currentCam, fps);
}

void uEyeCamera::unlockSequenceBuffer(INT* nMemID, char** pBuffer)
{
	is_UnlockSeqBuf(currentCam, *nMemID, *pBuffer);
}

void uEyeCamera::printSensorTypeAsString(WORD curSensorType)
{
	switch (curSensorType)
	{
		/*
		#define IS_SENSOR_INVALID           0x0000
		// CMOS Sensors
		#define IS_SENSOR_UI141X_M          0x0001      // VGA rolling shutter, monochrome
		#define IS_SENSOR_UI141X_C          0x0002      // VGA rolling shutter, color
		#define IS_SENSOR_UI144X_M          0x0003      // SXGA rolling shutter, monochrome
		#define IS_SENSOR_UI144X_C          0x0004      // SXGA rolling shutter, SXGA color

		#define IS_SENSOR_UI154X_M          0x0030      // SXGA rolling shutter, monochrome
		#define IS_SENSOR_UI154X_C          0x0031      // SXGA rolling shutter, color
		#define IS_SENSOR_UI145X_C          0x0008      // UXGA rolling shutter, color

		#define IS_SENSOR_UI146X_C          0x000a      // QXGA rolling shutter, color
		#define IS_SENSOR_UI148X_M          0x000b      // 5MP rolling shutter, monochrome
		#define IS_SENSOR_UI148X_C          0x000c      // 5MP rolling shutter, color

		#define IS_SENSOR_UI121X_M          0x0010      // VGA global shutter, monochrome
		#define IS_SENSOR_UI121X_C          0x0011      // VGA global shutter, VGA color
		#define IS_SENSOR_UI122X_M          0x0012      // WVGA global shutter, monochrome
		#define IS_SENSOR_UI122X_C          0x0013      // WVGA global shutter, color

		#define IS_SENSOR_UI164X_C          0x0015      // SXGA rolling shutter, color

		#define IS_SENSOR_UI155X_C          0x0017      // UXGA rolling shutter, color

		#define IS_SENSOR_UI1223_M          0x0018      // WVGA global shutter, monochrome
		#define IS_SENSOR_UI1223_C          0x0019      // WVGA global shutter, color

		#define IS_SENSOR_UI149X_M          0x003E      // 10MP rolling shutter, monochrome
		#define IS_SENSOR_UI149X_C          0x003d      // 10MP rolling shutter, color

		#define IS_SENSOR_UI1225_M          0x0022      // WVGA global shutter, monochrome, LE model
		#define IS_SENSOR_UI1225_C          0x0023      // WVGA global shutter, color, LE model

		#define IS_SENSOR_UI1645_C          0x0025      // SXGA rolling shutter, color, LE model
		#define IS_SENSOR_UI1555_C          0x0027      // UXGA rolling shutter, color, LE model
		#define IS_SENSOR_UI1545_M          0x0028      // SXGA rolling shutter, monochrome, LE model
		#define IS_SENSOR_UI1545_C          0x0029      // SXGA rolling shutter, color, LE model
		#define IS_SENSOR_UI1455_C          0x002B      // UXGA rolling shutter, color, LE model
		#define IS_SENSOR_UI1465_C          0x002D      // QXGA rolling shutter, color, LE model
		#define IS_SENSOR_UI1485_M          0x002E      // 5MP rolling shutter, monochrome, LE model
		#define IS_SENSOR_UI1485_C          0x002F      // 5MP rolling shutter, color, LE model
		#define IS_SENSOR_UI1495_M          0x0040      // 10MP rolling shutter, monochrome, LE model
		#define IS_SENSOR_UI1495_C          0x0041      // 10MP rolling shutter, color, LE model

		#define IS_SENSOR_UI112X_M          0x004A      // 0768x576, HDR sensor, monochrome
		#define IS_SENSOR_UI112X_C          0x004B      // 0768x576, HDR sensor, color

		#define IS_SENSOR_UI1008_M          0x004C
		#define IS_SENSOR_UI1008_C          0x004D

		#define IS_SENSOR_UI1005_M          0x020A
		#define IS_SENSOR_UI1005_C          0x020B

		#define IS_SENSOR_UI1240_M          0x0050      // SXGA global shutter, monochrome
		#define IS_SENSOR_UI1240_C          0x0051      // SXGA global shutter, color
		#define IS_SENSOR_UI1240_NIR        0x0062      // SXGA global shutter, NIR

		#define IS_SENSOR_UI1240LE_M        0x0054      // SXGA global shutter, monochrome, single board
		#define IS_SENSOR_UI1240LE_C        0x0055      // SXGA global shutter, color, single board
		#define IS_SENSOR_UI1240LE_NIR      0x0064      // SXGA global shutter, NIR, single board

		#define IS_SENSOR_UI1240ML_M        0x0066      // SXGA global shutter, monochrome, single board
		#define IS_SENSOR_UI1240ML_C        0x0067      // SXGA global shutter, color, single board
		#define IS_SENSOR_UI1240ML_NIR      0x0200      // SXGA global shutter, NIR, single board

		#define IS_SENSOR_UI1243_M_SMI      0x0078
		#define IS_SENSOR_UI1243_C_SMI      0x0079

		#define IS_SENSOR_UI1543_M          0x0032      // SXGA rolling shutter, monochrome, single board
		#define IS_SENSOR_UI1543_C          0x0033      // SXGA rolling shutter, color, single board

		#define IS_SENSOR_UI1544_M          0x003A      // SXGA rolling shutter, monochrome, single board
		#define IS_SENSOR_UI1544_C          0x003B      // SXGA rolling shutter, color, single board
		#define IS_SENSOR_UI1543_M_WO       0x003C      // SXGA rolling shutter, monochrome, single board
		#define IS_SENSOR_UI1543_C_WO       0x003D      // SXGA rolling shutter, color, single board
		#define IS_SENSOR_UI1453_C          0x0035      // UXGA rolling shutter, color, single board
		#define IS_SENSOR_UI1463_C          0x0037      // QXGA rolling shutter, color, single board
		#define IS_SENSOR_UI1483_M          0x0038      // QSXG rolling shutter, monochrome, single board
		#define IS_SENSOR_UI1483_C          0x0039      // QSXG rolling shutter, color, single board
		#define IS_SENSOR_UI1493_M          0x004E      // 10Mp rolling shutter, monochrome, single board
		#define IS_SENSOR_UI1493_C          0x004F      // 10MP rolling shutter, color, single board

		#define IS_SENSOR_UI1463_M_WO       0x0044      // QXGA rolling shutter, monochrome, single board
		#define IS_SENSOR_UI1463_C_WO       0x0045      // QXGA rolling shutter, color, single board

		#define IS_SENSOR_UI1553_C_WN       0x0047      // UXGA rolling shutter, color, single board
		#define IS_SENSOR_UI1483_M_WO       0x0048      // QSXGA rolling shutter, monochrome, single board
		#define IS_SENSOR_UI1483_C_WO       0x0049      // QSXGA rolling shutter, color, single board

		#define IS_SENSOR_UI1580_M          0x005A      // 5MP rolling shutter, monochrome
		#define IS_SENSOR_UI1580_C          0x005B      // 5MP rolling shutter, color
		#define IS_SENSOR_UI1580LE_M        0x0060      // 5MP rolling shutter, monochrome, single board
		#define IS_SENSOR_UI1580LE_C        0x0061      // 5MP rolling shutter, color, single board

		#define IS_SENSOR_UI1360M           0x0068      // 2.2MP global shutter, monochrome
		#define IS_SENSOR_UI1360C           0x0069      // 2.2MP global shutter, color
		#define IS_SENSOR_UI1360NIR         0x0212      // 2.2MP global shutter, NIR

		#define IS_SENSOR_UI1370M           0x006A      // 4.2MP global shutter, monochrome
		#define IS_SENSOR_UI1370C           0x006B      // 4.2MP global shutter, color
		#define IS_SENSOR_UI1370NIR         0x0214      // 4.2MP global shutter, NIR

		#define IS_SENSOR_UI1250_M          0x006C      // 2MP global shutter, monochrome
		#define IS_SENSOR_UI1250_C          0x006D      // 2MP global shutter, color
		#define IS_SENSOR_UI1250_NIR        0x006E      // 2MP global shutter, NIR

		#define IS_SENSOR_UI1250LE_M        0x0070      // 2MP global shutter, monochrome, single board
		#define IS_SENSOR_UI1250LE_C        0x0071      // 2MP global shutter, color, single board
		#define IS_SENSOR_UI1250LE_NIR      0x0072      // 2MP global shutter, NIR, single board

		#define IS_SENSOR_UI1250ML_M        0x0074      // 2MP global shutter, monochrome, single board
		#define IS_SENSOR_UI1250ML_C        0x0075      // 2MP global shutter, color, single board
		#define IS_SENSOR_UI1250ML_NIR      0x0202      // 2MP global shutter, NIR, single board

		#define IS_SENSOR_XS                0x020B      // 5MP rolling shutter, color

		#define IS_SENSOR_UI1493_M_AR       0x0204
		#define IS_SENSOR_UI1493_C_AR       0x0205

		#define IS_SENSOR_UI1060_M          0x021A      // 2.3MP global shutter, monochrome
		#define IS_SENSOR_UI1060_C          0x021B      // 2.3MP global shutter, color

		#define IS_SENSOR_UI1013XC          0x021D      // 13MP, color

		#define IS_SENSOR_UI1140M           0x021E      // 1.3MP global shutter, monochrome
		#define IS_SENSOR_UI1140C           0x021F      // 1.3MP global shutter, color
		#define IS_SENSOR_UI1140NIR         0x0220      // 1.3MP global shutter, NIR

		#define IS_SENSOR_UI1590M           0x0222      // 18MP rolling shutter, monochrome
		#define IS_SENSOR_UI1590C           0x0223      // 18MP rolling shutter, color

		#define IS_SENSOR_UI1260_M          0x0226      // 2.3MP global shutter, monochrome
		#define IS_SENSOR_UI1260_C          0x0227      // 2.3MP global shutter, color

		#define IS_SENSOR_UI1130_M          0x022A      // SVGA global shutter, monochrome
		#define IS_SENSOR_UI1130_C          0x022B      // SVGA global shutter, color

		#define IS_SENSOR_UI1160_M          0x022C      // 2.3MP global shutter, monochrome
		#define IS_SENSOR_UI1160_C          0x022D      // 2.3MP global shutter, color

		#define IS_SENSOR_UI1180_M          0x022E      // 5.3MP global shutter, monochrome
		#define IS_SENSOR_UI1180_C          0x022F      // 5.3MP global shutter, color

		#define IS_SENSOR_UI1080_M          0x0230      // 5MP global shutter, monochrome
		#define IS_SENSOR_UI1080_C          0x0231      // 5MP global shutter, color

		#define IS_SENSOR_UI1280_M          0x0232      // 5MP global shutter, monochrome
		#define IS_SENSOR_UI1280_C          0x0233      // 5MP global shutter, color

		#define IS_SENSOR_UI1860_M          0x0234      // 2MP rolling shutter, monochrome
		#define IS_SENSOR_UI1860_C          0x0235      // 2MP rolling shutter, color

		#define IS_SENSOR_UI1880_M          0x0236      // 6MP rolling shutter, monochrome
		#define IS_SENSOR_UI1880_C          0x0237      // 6MP rolling shutter, color

		#define IS_SENSOR_UI1270_M          0x0238      // 3.2MP global shutter, monochrome
		#define IS_SENSOR_UI1270_C          0x0239      // 3.2MP global shutter, color

		#define IS_SENSOR_UI1070_M          0x023A      // 3.2MP global shutter, monochrome
		#define IS_SENSOR_UI1070_C          0x023B      // 3.2MP global shutter, color

		#define IS_SENSOR_UI1130LE_M        0x023C      // SVGA global shutter, monochrome
		#define IS_SENSOR_UI1130LE_C        0x023D      // SVGA global shutter, color

		// CCD Sensors
		#define IS_SENSOR_UI223X_M          0x0080      // Sony CCD sensor - XGA monochrome
		#define IS_SENSOR_UI223X_C          0x0081      // Sony CCD sensor - XGA color

		#define IS_SENSOR_UI241X_M          0x0082      // Sony CCD sensor - VGA monochrome
		#define IS_SENSOR_UI241X_C          0x0083      // Sony CCD sensor - VGA color

		#define IS_SENSOR_UI234X_M          0x0084      // Sony CCD sensor - SXGA monochrome
		#define IS_SENSOR_UI234X_C          0x0085      // Sony CCD sensor - SXGA color

		#define IS_SENSOR_UI221X_M          0x0088      // Sony CCD sensor - VGA monochrome
		#define IS_SENSOR_UI221X_C          0x0089      // Sony CCD sensor - VGA color

		#define IS_SENSOR_UI231X_M          0x0090      // Sony CCD sensor - VGA monochrome
		#define IS_SENSOR_UI231X_C          0x0091      // Sony CCD sensor - VGA color

		#define IS_SENSOR_UI222X_M          0x0092      // Sony CCD sensor - CCIR / PAL monochrome
		#define IS_SENSOR_UI222X_C          0x0093      // Sony CCD sensor - CCIR / PAL color

		#define IS_SENSOR_UI224X_M          0x0096      // Sony CCD sensor - SXGA monochrome
		#define IS_SENSOR_UI224X_C          0x0097      // Sony CCD sensor - SXGA color

		#define IS_SENSOR_UI225X_M          0x0098      // Sony CCD sensor - UXGA monochrome
		#define IS_SENSOR_UI225X_C          0x0099      // Sony CCD sensor - UXGA color

		#define IS_SENSOR_UI214X_M          0x009A      // Sony CCD sensor - SXGA monochrome
		#define IS_SENSOR_UI214X_C          0x009B      // Sony CCD sensor - SXGA color

		#define IS_SENSOR_UI228X_M          0x009C      // Sony CCD sensor - QXGA monochrome
		#define IS_SENSOR_UI228X_C          0x009D      // Sony CCD sensor - QXGA color

		#define IS_SENSOR_UI241X_M_R2       0x0182      // Sony CCD sensor - VGA monochrome
		#define IS_SENSOR_UI251X_M          0x0182      // Sony CCD sensor - VGA monochrome
		#define IS_SENSOR_UI241X_C_R2       0x0183      // Sony CCD sensor - VGA color
		#define IS_SENSOR_UI251X_C          0x0183      // Sony CCD sensor - VGA color

		#define IS_SENSOR_UI2130_M          0x019E      // Sony CCD sensor - WXGA monochrome
		#define IS_SENSOR_UI2130_C          0x019F      // Sony CCD sensor - WXGA color

		#define IS_SENSOR_PASSIVE_MULTICAST 0x0F00
		*/
	}
}

void uEyeCamera::printColorMode(int curColorMode)
{
	switch (curColorMode)
	{
	case IS_CM_MONO16:
		qDebug() << "ColorMode: Graustufen(16), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_MONO12:
		qDebug() << "ColorMode: Graustufen(12), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_MONO10:
		qDebug() << "ColorMode: Graustufen(10), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_MONO8:
		qDebug() << "ColorMode: Graustufen(8), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_SENSOR_RAW16:
		qDebug() <<
			"ColorMode: Raw Bayer(16), unveränderte Sensorrohdaten fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion nicht aktiv.";
		break;
	case IS_CM_SENSOR_RAW12:
		qDebug() <<
			"ColorMode: Raw Bayer(12), unveränderte Sensorrohdaten fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion nicht aktiv.";
		break;
	case IS_CM_SENSOR_RAW10:
		qDebug() <<
			"ColorMode: Raw Bayer(10), unveränderte Sensorrohdaten fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion nicht aktiv.";
		break;
	case IS_CM_SENSOR_RAW8:
		qDebug() <<
			"Raw Bayer(8), unveränderte Sensorrohdaten fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion nicht aktiv.";
		break;
	case IS_CM_RGB12_UNPACKED:
		qDebug() << "ColorMode: Ungepacktes RGB(12 12 12), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_RGB10_UNPACKED:
		qDebug() << "ColorMode: Ungepacktes RGB(10 10 10), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_RGB10_PACKED:
		qDebug() << "ColorMode: RGB(10 10 10), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_RGB8_PACKED:
		qDebug() << "ColorMode: RGB(8 8 8), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_RGBA12_UNPACKED:
		qDebug() << "ColorMode: Ungepacktes RGB(12 12 12), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_RGBA8_PACKED:
		qDebug() << "ColorMode: RGB(8 8 8), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_RGBY8_PACKED:
		qDebug() << "ColorMode: RGBY(8 8 8 8), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_BGR12_UNPACKED:
		qDebug() << "ColorMode: Ungepacktes BGR(12 12 12), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_BGR10_UNPACKED:
		qDebug() << "ColorMode: Ungepacktes BGR(10 10 10), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_BGR10_PACKED:
		qDebug() << "ColorMode: BGR(10 10 10), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_BGR8_PACKED:
		qDebug() << "ColorMode: BGR(8 8 8), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_BGRA12_UNPACKED:
		qDebug() << "ColorMode: Ungepacktes BGR(12 12 12), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_BGRA8_PACKED:
		qDebug() << "ColorMode: BGR(8 8 8), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_BGRY8_PACKED:
		qDebug() << "ColorMode: BGRY(8 8 8), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_RGB8_PLANAR:
		qDebug() << "ColorMode: Planares RGB(8) fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_BGR565_PACKED:
		qDebug() << "ColorMode: BGR(5 6 5), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_BGR5_PACKED:
		qDebug() << "ColorMode: BGR(5 5 5), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_UYVY_PACKED:
		qDebug() << "ColorMode: YUV 4:2 : 2 (8 8), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_CBYCRY_PACKED:
		qDebug() << "ColorMode: YCbCr 4 : 2 : 2 (8 8), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	case IS_CM_PREFER_PACKED_SOURCE_FORMAT:
		qDebug() << "ColorMode: YUV 4:2 : 2 (8 8), fuer Monochrom - und Farbkameras, LUT - / Gamma - Funktion aktiv.";
		break;
	default:
		qDebug() << "not detected";
		break;
	}
}

void uEyeCamera::printErrorCode(int error)
{
	/*
		#define IS_NO_SUCCESS                        -1   // function call failed
		#define IS_SUCCESS                            0   // function call succeeded
		#define IS_INVALID_CAMERA_HANDLE              1   // camera handle is not valid or zero
		#define IS_INVALID_HANDLE                     1   // a handle other than the camera handle is invalid

		#define IS_IO_REQUEST_FAILED                  2   // an io request to the driver failed
		#define IS_CANT_OPEN_DEVICE                   3   // returned by is_InitCamera
		#define IS_CANT_CLOSE_DEVICE                  4
		#define IS_CANT_SETUP_MEMORY                  5
		#define IS_NO_HWND_FOR_ERROR_REPORT           6
		#define IS_ERROR_MESSAGE_NOT_CREATED          7
		#define IS_ERROR_STRING_NOT_FOUND             8
		#define IS_HOOK_NOT_CREATED                   9
		#define IS_TIMER_NOT_CREATED                 10
		#define IS_CANT_OPEN_REGISTRY                11
		#define IS_CANT_READ_REGISTRY                12
		#define IS_CANT_VALIDATE_BOARD               13
		#define IS_CANT_GIVE_BOARD_ACCESS            14
		#define IS_NO_IMAGE_MEM_ALLOCATED            15
		#define IS_CANT_CLEANUP_MEMORY               16
		#define IS_CANT_COMMUNICATE_WITH_DRIVER      17
		#define IS_FUNCTION_NOT_SUPPORTED_YET        18
		#define IS_OPERATING_SYSTEM_NOT_SUPPORTED    19

		#define IS_INVALID_VIDEO_IN                  20
		#define IS_INVALID_IMG_SIZE                  21
		#define IS_INVALID_ADDRESS                   22
		#define IS_INVALID_VIDEO_MODE                23
		#define IS_INVALID_AGC_MODE                  24
		#define IS_INVALID_GAMMA_MODE                25
		#define IS_INVALID_SYNC_LEVEL                26
		#define IS_INVALID_CBARS_MODE                27
		#define IS_INVALID_COLOR_MODE                28
		#define IS_INVALID_SCALE_FACTOR              29
		#define IS_INVALID_IMAGE_SIZE                30
		#define IS_INVALID_IMAGE_POS                 31
		#define IS_INVALID_CAPTURE_MODE              32
		#define IS_INVALID_RISC_PROGRAM              33
		#define IS_INVALID_BRIGHTNESS                34
		#define IS_INVALID_CONTRAST                  35
		#define IS_INVALID_SATURATION_U              36
		#define IS_INVALID_SATURATION_V              37
		#define IS_INVALID_HUE                       38
		#define IS_INVALID_HOR_FILTER_STEP           39
		#define IS_INVALID_VERT_FILTER_STEP          40
		#define IS_INVALID_EEPROM_READ_ADDRESS       41
		#define IS_INVALID_EEPROM_WRITE_ADDRESS      42
		#define IS_INVALID_EEPROM_READ_LENGTH        43
		#define IS_INVALID_EEPROM_WRITE_LENGTH       44
		#define IS_INVALID_BOARD_INFO_POINTER        45
		#define IS_INVALID_DISPLAY_MODE              46
		#define IS_INVALID_ERR_REP_MODE              47
		#define IS_INVALID_BITS_PIXEL                48
		#define IS_INVALID_MEMORY_POINTER            49

		#define IS_FILE_WRITE_OPEN_ERROR             50
		#define IS_FILE_READ_OPEN_ERROR              51
		#define IS_FILE_READ_INVALID_BMP_ID          52
		#define IS_FILE_READ_INVALID_BMP_SIZE        53
		#define IS_FILE_READ_INVALID_BIT_COUNT       54
		#define IS_WRONG_KERNEL_VERSION              55

		#define IS_RISC_INVALID_XLENGTH              60
		#define IS_RISC_INVALID_YLENGTH              61
		#define IS_RISC_EXCEED_IMG_SIZE              62

			// DirectDraw Mode errors
		#define IS_DD_MAIN_FAILED                    70
		#define IS_DD_PRIMSURFACE_FAILED             71
		#define IS_DD_SCRN_SIZE_NOT_SUPPORTED        72
		#define IS_DD_CLIPPER_FAILED                 73
		#define IS_DD_CLIPPER_HWND_FAILED            74
		#define IS_DD_CLIPPER_CONNECT_FAILED         75
		#define IS_DD_BACKSURFACE_FAILED             76
		#define IS_DD_BACKSURFACE_IN_SYSMEM          77
		#define IS_DD_MDL_MALLOC_ERR                 78
		#define IS_DD_MDL_SIZE_ERR                   79
		#define IS_DD_CLIP_NO_CHANGE                 80
		#define IS_DD_PRIMMEM_NULL                   81
		#define IS_DD_BACKMEM_NULL                   82
		#define IS_DD_BACKOVLMEM_NULL                83
		#define IS_DD_OVERLAYSURFACE_FAILED          84
		#define IS_DD_OVERLAYSURFACE_IN_SYSMEM       85
		#define IS_DD_OVERLAY_NOT_ALLOWED            86
		#define IS_DD_OVERLAY_COLKEY_ERR             87
		#define IS_DD_OVERLAY_NOT_ENABLED            88
		#define IS_DD_GET_DC_ERROR                   89
		#define IS_DD_DDRAW_DLL_NOT_LOADED           90
		#define IS_DD_THREAD_NOT_CREATED             91
		#define IS_DD_CANT_GET_CAPS                  92
		#define IS_DD_NO_OVERLAYSURFACE              93
		#define IS_DD_NO_OVERLAYSTRETCH              94
		#define IS_DD_CANT_CREATE_OVERLAYSURFACE     95
		#define IS_DD_CANT_UPDATE_OVERLAYSURFACE     96
		#define IS_DD_INVALID_STRETCH                97

		#define IS_EV_INVALID_EVENT_NUMBER          100
		#define IS_INVALID_MODE                     101
		#define IS_CANT_FIND_FALCHOOK               102
		#define IS_CANT_FIND_HOOK                   102
		#define IS_CANT_GET_HOOK_PROC_ADDR          103
		#define IS_CANT_CHAIN_HOOK_PROC             104
		#define IS_CANT_SETUP_WND_PROC              105
		#define IS_HWND_NULL                        106
		#define IS_INVALID_UPDATE_MODE              107
		#define IS_NO_ACTIVE_IMG_MEM                108
		#define IS_CANT_INIT_EVENT                  109
		#define IS_FUNC_NOT_AVAIL_IN_OS             110
		#define IS_CAMERA_NOT_CONNECTED             111
		#define IS_SEQUENCE_LIST_EMPTY              112
		#define IS_CANT_ADD_TO_SEQUENCE             113
		#define IS_LOW_OF_SEQUENCE_RISC_MEM         114
		#define IS_IMGMEM2FREE_USED_IN_SEQ          115
		#define IS_IMGMEM_NOT_IN_SEQUENCE_LIST      116
		#define IS_SEQUENCE_BUF_ALREADY_LOCKED      117
		#define IS_INVALID_DEVICE_ID                118
		#define IS_INVALID_BOARD_ID                 119
		#define IS_ALL_DEVICES_BUSY                 120
		#define IS_HOOK_BUSY                        121
		#define IS_TIMED_OUT                        122
		#define IS_NULL_POINTER                     123
		#define IS_WRONG_HOOK_VERSION               124
		#define IS_INVALID_PARAMETER                125   // a parameter specified was invalid
		#define IS_NOT_ALLOWED                      126
		#define IS_OUT_OF_MEMORY                    127
		#define IS_INVALID_WHILE_LIVE               128
		#define IS_ACCESS_VIOLATION                 129   // an internal exception occurred
		#define IS_UNKNOWN_ROP_EFFECT               130
		#define IS_INVALID_RENDER_MODE              131
		#define IS_INVALID_THREAD_CONTEXT           132
		#define IS_NO_HARDWARE_INSTALLED            133
		#define IS_INVALID_WATCHDOG_TIME            134
		#define IS_INVALID_WATCHDOG_MODE            135
		#define IS_INVALID_PASSTHROUGH_IN           136
		#define IS_ERROR_SETTING_PASSTHROUGH_IN     137
		#define IS_FAILURE_ON_SETTING_WATCHDOG      138
		#define IS_NO_USB20                         139   // the usb port doesnt support usb 2.0
		#define IS_CAPTURE_RUNNING                  140   // there is already a capture running

		#define IS_MEMORY_BOARD_ACTIVATED           141   // operation could not execute while mboard is enabled
		#define IS_MEMORY_BOARD_DEACTIVATED         142   // operation could not execute while mboard is disabled
		#define IS_NO_MEMORY_BOARD_CONNECTED        143   // no memory board connected
		#define IS_TOO_LESS_MEMORY                  144   // image size is above memory capacity
		#define IS_IMAGE_NOT_PRESENT                145   // requested image is no longer present in the camera
		#define IS_MEMORY_MODE_RUNNING              146
		#define IS_MEMORYBOARD_DISABLED             147

		#define IS_TRIGGER_ACTIVATED                148   // operation could not execute while trigger is enabled
		#define IS_WRONG_KEY                        150
		#define IS_CRC_ERROR                        151
		#define IS_NOT_YET_RELEASED                 152   // this feature is not available yet
		#define IS_NOT_CALIBRATED                   153   // the camera is not calibrated
		#define IS_WAITING_FOR_KERNEL               154   // a request to the kernel exceeded
		#define IS_NOT_SUPPORTED                    155   // operation mode is not supported
		#define IS_TRIGGER_NOT_ACTIVATED            156   // operation could not execute while trigger is disabled
		#define IS_OPERATION_ABORTED                157
		#define IS_BAD_STRUCTURE_SIZE               158
		#define IS_INVALID_BUFFER_SIZE              159
		#define IS_INVALID_PIXEL_CLOCK              160
		#define IS_INVALID_EXPOSURE_TIME            161
		#define IS_AUTO_EXPOSURE_RUNNING            162
		#define IS_CANNOT_CREATE_BB_SURF            163   // error creating backbuffer surface
		#define IS_CANNOT_CREATE_BB_MIX             164   // backbuffer mixer surfaces can not be created
		#define IS_BB_OVLMEM_NULL                   165   // backbuffer overlay mem could not be locked
		#define IS_CANNOT_CREATE_BB_OVL             166   // backbuffer overlay mem could not be created
		#define IS_NOT_SUPP_IN_OVL_SURF_MODE        167   // function not supported in overlay surface mode
		#define IS_INVALID_SURFACE                  168   // surface invalid
		#define IS_SURFACE_LOST                     169   // surface has been lost
		#define IS_RELEASE_BB_OVL_DC                170   // error releasing backbuffer overlay DC
		#define IS_BB_TIMER_NOT_CREATED             171   // backbuffer timer could not be created
		#define IS_BB_OVL_NOT_EN                    172   // backbuffer overlay has not been enabled
		#define IS_ONLY_IN_BB_MODE                  173   // only possible in backbuffer mode
		#define IS_INVALID_COLOR_FORMAT             174   // invalid color format
		#define IS_INVALID_WB_BINNING_MODE          175   // invalid binning mode for AWB
		#define IS_INVALID_I2C_DEVICE_ADDRESS       176   // invalid I2C device address
		#define IS_COULD_NOT_CONVERT                177   // current image couldn't be converted
		#define IS_TRANSFER_ERROR                   178   // transfer failed
		#define IS_PARAMETER_SET_NOT_PRESENT        179   // the parameter set is not present
		#define IS_INVALID_CAMERA_TYPE              180   // the camera type in the ini file doesn't match
		#define IS_INVALID_HOST_IP_HIBYTE           181   // HIBYTE of host address is invalid
		#define IS_CM_NOT_SUPP_IN_CURR_DISPLAYMODE  182   // color mode is not supported in the current display mode
		#define IS_NO_IR_FILTER                     183
		#define IS_STARTER_FW_UPLOAD_NEEDED         184   // device starter firmware is not compatible

		#define IS_DR_LIBRARY_NOT_FOUND                     185   // the DirectRender library could not be found
		#define IS_DR_DEVICE_OUT_OF_MEMORY                  186   // insufficient graphics adapter video memory
		#define IS_DR_CANNOT_CREATE_SURFACE                 187   // the image or overlay surface could not be created
		#define IS_DR_CANNOT_CREATE_VERTEX_BUFFER           188   // the vertex buffer could not be created
		#define IS_DR_CANNOT_CREATE_TEXTURE                 189   // the texture could not be created
		#define IS_DR_CANNOT_LOCK_OVERLAY_SURFACE           190   // the overlay surface could not be locked
		#define IS_DR_CANNOT_UNLOCK_OVERLAY_SURFACE         191   // the overlay surface could not be unlocked
		#define IS_DR_CANNOT_GET_OVERLAY_DC                 192   // cannot get the overlay surface DC
		#define IS_DR_CANNOT_RELEASE_OVERLAY_DC             193   // cannot release the overlay surface DC
		#define IS_DR_DEVICE_CAPS_INSUFFICIENT              194   // insufficient graphics adapter capabilities
		#define IS_INCOMPATIBLE_SETTING                     195   // Operation is not possible because of another incompatible setting
		#define IS_DR_NOT_ALLOWED_WHILE_DC_IS_ACTIVE        196   // user App still has DC handle.
		#define IS_DEVICE_ALREADY_PAIRED                    197   // The device is already paired
		#define IS_SUBNETMASK_MISMATCH                      198   // The subnetmasks of the device and the adapter differ
		#define IS_SUBNET_MISMATCH                          199   // The subnets of the device and the adapter differ
		#define IS_INVALID_IP_CONFIGURATION                 200   // The IP configuation of the device is invalid
		#define IS_DEVICE_NOT_COMPATIBLE                    201   // The device is incompatible to the driver
		#define IS_NETWORK_FRAME_SIZE_INCOMPATIBLE          202   // The frame size settings of the device and the network adapter are incompatible
		#define IS_NETWORK_CONFIGURATION_INVALID            203   // The network adapter configuration is invalid
		#define IS_ERROR_CPU_IDLE_STATES_CONFIGURATION      204   // The setting of the CPU idle state configuration failed
		#define IS_DEVICE_BUSY                              205   // The device is busy. The operation must be executed again later.
		#define IS_SENSOR_INITIALIZATION_FAILED             206   // The sensor initialization failed

		*/
}

// value between 1 and 100
void uEyeCamera::setGain(int newVal)
{
	INT nGain = newVal;

	/* Get sensor source gain range */
	IS_RANGE_S32 rangeSourceGain;
	INT nRet = is_DeviceFeature(currentCam, IS_DEVICE_FEATURE_CMD_GET_SENSOR_SOURCE_GAIN_RANGE, (void*)&rangeSourceGain,
	                            sizeof(rangeSourceGain));

	if (nRet == IS_SUCCESS)
	{
		/* Set sensor source gain to max */
		if (nGain > rangeSourceGain.s32Max)
		{
			qDebug() << "Source Gain Max value is: " << rangeSourceGain.s32Max;
			nGain = rangeSourceGain.s32Max;
		}

		nRet = is_DeviceFeature(currentCam, IS_DEVICE_FEATURE_CMD_SET_SENSOR_SOURCE_GAIN, (void*)&nGain, sizeof(nGain));

		qDebug() << "Gain set to: " << nGain;
	}
}

// value between 100 and 220
void uEyeCamera::setGamma(int newVal)
{
	/* gamma val is betwwen 1.0 and 2.2 */
	/* for this: gamma 1.0 => set to 100 
					   2.2 => set to 220 */

	INT nGamma = newVal;
	INT nRet = is_Gamma(currentCam, IS_GAMMA_CMD_SET, (void*)&nGamma, sizeof(nGamma));

	if (nRet == IS_SUCCESS)
	{
		qDebug() << "Gamma is set to: " << nGamma;
	}
}

void uEyeCamera::setGainBoost()
{
	INT nRet = is_SetGainBoost(currentCam, IS_SET_GAINBOOST_ON);
	if (nRet == IS_SUCCESS)
	{
		qDebug() << "Hardware Gain Boost activated.";
	}


	if (nRet == IS_NO_SUCCESS)
	{
		qDebug() << "Hardware Gain Boost not activated.";
	}
}


void uEyeCamera::setHWGain(int val)
{
	INT nRet = is_SetHardwareGain(currentCam, val, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER);


	if (nRet == IS_SUCCESS)
	{
		qDebug() << "Hardware Gain set to: " << val;
	}


	if (nRet == IS_NO_SUCCESS)
	{
		qDebug() << "Hardware Gain NOT set";
	}
}
