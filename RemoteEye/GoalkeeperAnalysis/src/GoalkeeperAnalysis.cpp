#include "GoalkeeperAnalysis.h"
#include "ui_GoalkeeperAnalysis.h"

#include <fstream>

#include "constants.h"


#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QDesktopWidget>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <utility>
#include <boost/filesystem.hpp>

#include "algo_fwd.h"

#include "kalman.h"

#include "VideoCaptureSession.h"
#include "IntermediateTypes.h"
#include "VideoFilePollingThread.h"
#include "VideoWriterThread.h"
#include "utils.h"
#include "DetectorCode.h"
#include "CalibrationDialog.h"
#include "CalibrationPatternGenerators.h"
#include "GazeEstimationAdapters.hpp"
#include "Calibration.h"
#include "OutputMethods.h"
#include "uEyeCamera.h"
#include "UEyePollingThread.h"
#include "LiveGazeDisplay.h"
#include "CalibrationTrainer.h"
#include "ProfilingMonitor.h"
#include "InputFeaturesDisplay.h"


using namespace gazeestimation;


void GoalkeeperAnalysis::setupDefaultSetup()
{
	/// Important: it is insufficient to just change some setup parameters here.
	/// The modelling of the scene in GazeEstimationAdapters.hpp and the calibration procedure
	/// (which supplies the default vaules & bounds for the parameters that we calibrate against,)
	/// must be changed as well if there are changes to the scene that go beyond changing the value
	/// of an indivudal parameter.
	EyeAndCameraParameters parameters;
	parameters.alpha = deg_to_rad(-5);
	parameters.beta = deg_to_rad(1.5);
	parameters.R = 0.78;
	parameters.K = 0.42;
	parameters.n1 = 1.3375;
	parameters.n2 = 1;
	parameters.D = 0.53;

	// keeping in mind that wcs has its origin at the camera position for these

	// MANUELLE EINGABE: Kamera position in Referenz zum linken oberen Bildschirm eck
	//const Vec3 actual_camera_position = make_vec3(24.5, -35, 10);
	const Vec3 actual_camera_position = make_vec3(26.0, -35, 4);
	wcs_offset = -actual_camera_position;


	gazeestimation::PinholeCameraModel camera;
	camera.principal_point_x = 399.5;
	camera.principal_point_y = 299.5;
	camera.pixel_size_cm_x = 4.8 * 1e-6 * 100;
	camera.pixel_size_cm_y = 4.8 * 1e-6 * 100;
	//camera.effective_focal_length_cm = 1.235;
	//camera.effective_focal_length_cm = 1.600;


	camera.effective_focal_length_cm = 1.61969;
	camera.position = actual_camera_position + wcs_offset;
	//camera.set_camera_angles(deg_to_rad(16), 0, 0);


	// CAMERA WINKEL
	camera.set_camera_angles(deg_to_rad(-28), 0, 0);	//25
	parameters.cameras.push_back(camera);

	// the lights rotate with the camera, see also sixVariableCalibrationApplicator. 
	std::vector<Vec3> lights;
	//lights.push_back(camera.ccs_to_wcs(make_vec3(25.0, 2.0, 2.0)));
	//lights.push_back(camera.ccs_to_wcs(make_vec3(0, -8.5, -3.0)));
	//lights.push_back(camera.ccs_to_wcs(make_vec3(-25.0, 2.0, 2.0)));

	// MANUELLE EINGABE : Position der LEDs in Refernz zur Kamera
	lights.push_back(camera.ccs_to_wcs(make_vec3(25.1, 2.5, -1.0)));
	lights.push_back(camera.ccs_to_wcs(make_vec3(0, -8.5, -2.0)));
	lights.push_back(camera.ccs_to_wcs(make_vec3(-25.1, 2.5, -1.0)));

	//lights.push_back(camera.ccs_to_wcs(make_vec3(25.0, 2.5, 0.5)));
	//lights.push_back(camera.ccs_to_wcs(make_vec3(0, -8.5, -3.0)));
	//lights.push_back(camera.ccs_to_wcs(make_vec3(-25.0, 2.5, 0.5)));



	parameters.light_positions = lights;

	parameters.distance_to_camera_estimate = 4;	//  9

	// additional scene parameters to get poi in pixels
	//const double display_surface_size_cm_x = 51.84;
	//const double display_surface_size_cm_y = 32.40;

	// MANUELLE EINGABE: Physikalische Größe des Bildschirms
	const double display_surface_size_cm_x = 52.0;
	const double display_surface_size_cm_y = 32.70;


	// MANUELLE EINGABE: Auflösung muss an Bildschirm angepasst werden
	const double screen_resolution_x = 1920;
	const double screen_resolution_y = 1200;
	
	const double screen_pixel_size_x = display_surface_size_cm_x / screen_resolution_x;
	const double screen_pixel_size_y = display_surface_size_cm_y / screen_resolution_y;

	z_shift = -actual_camera_position[2];

	screen_pixel_size_cm = make_vec2(screen_pixel_size_x, screen_pixel_size_y);
	screen_resolution = make_vec2(screen_resolution_x, screen_resolution_y);
	setup_parameters_left = parameters;
	setup_parameters_right = parameters;
	// left eye alpha is different in direction from right eye alpha
	setup_parameters_right.alpha = deg_to_rad(5);
}

void GoalkeeperAnalysis::setupCapture(OperationMode target_mode)
{
	
	if(display_found_features && input_features_display == nullptr)
	{
		input_features_display.reset(new InputFeaturesDisplay(30));
		connect(input_features_display.get(), SIGNAL(ended()), this, SLOT(stopAllOngoing()));
		input_features_display->show();
	}
	


#if defined _DEBUG || defined REL_WITH_DEBUG_INFO
	if(use_video_file_in_debug)
	{
		qWarning() << "Using static video file source.";
		if(target_mode == OperationMode::CALIBRATION || target_mode == OperationMode::CALIBRATION_ACCURACY)
		{
			auto polling = new VideoFilePollingThread("C:/Work/calibration.avi", true,
				std::bind(&GoalkeeperAnalysis::receiveFrame, this, std::placeholders::_1));
			video_frame_size = polling->videoSize();
			video_framerate = polling->framerate();
			capture.reset(new VideoCaptureSession(polling));
			return;
		}

		auto polling = new VideoFilePollingThread("C:/Work/calibration.avi", true,
			std::bind(&GoalkeeperAnalysis::receiveFrame, this, std::placeholders::_1));
		video_frame_size = polling->videoSize();
		video_framerate = polling->framerate();
		qDebug() << "static video file source with " << video_framerate << "fps";
		capture.reset(new VideoCaptureSession(polling));
		return;
	}
#endif

	// CAMERA IMAGE SIZE
	//cv::Size output_size = cv::Size(camera->sensorInfo.nMaxWidth, camera->sensorInfo.nMaxHeight);

	cv::Size output_size = cv::Size(camera->actualWidth, camera->actualHeight);
	if(target_mode == OperationMode::SAVING_FULL_IMAGE)
	{
		output_size = cv::Size(camera->sensorInfo.nMaxWidth, camera->sensorInfo.nMaxHeight);
	}

	this->video_frame_size = output_size;

	if(target_mode == OperationMode::CALIBRATION)
	{
		camera->setFrameRate(500); // 575
		camera->setExposure(target_exposure); //  1.9			
		camera->brightenUpCapturing();		
		this->video_framerate = 500;
	}
	else if(target_mode == OperationMode::CALIBRATION_ACCURACY)
	{
		camera->setFrameRate(500); // 25		Arduino 2500	25 - 6.0
		camera->setExposure(target_exposure); //4.0			// 5
		camera->brightenUpCapturing();
		this->video_framerate = 500;
	}
	else
	{
		camera->setFrameRate(500); // 25		Arduino 2500	25 - 6.0
		camera->setExposure(1.9); //4.0			// 5	
		camera->brightenUpCapturing();		// CHANGED 24.05.
		this->video_framerate = 500;
	}

	auto polling = new UEyePollingThread(camera.get(), std::bind(&GoalkeeperAnalysis::receiveFrame, this, std::placeholders::_1));
	capture.reset(new VideoCaptureSession(polling));
}



CalibrationTrainer* GoalkeeperAnalysis::createTrainer(PupilDetection* wrapper)
{
	if (BoreWrapper* w = dynamic_cast<BoreWrapper*>(wrapper)) {
		qDebug() << "Creating BORE trainer";
		return createTrainer(w);
	}
	else
		return new NoTraining();
}

CalibrationTrainer* GoalkeeperAnalysis::createTrainer(BoreWrapper* wrapper)
{
	//CalibrationTrainer::FeatureDetection detection = [this](const cv::Mat& frame) {return DetectedFeatures(detectRaw(frame)); };
	CalibrationTrainer::FeatureDetectionSparse detection = [this](const SparseInputImage& frame) {return DetectedFeatures(detectRaw(frame)); };
	BoreTrainer::EyeSeparation eye_separation = [this](const cv::Mat& frame)
	{
		cv::Mat left_eye, right_eye;
		bool left = cutEyeRegion(frame, left_eye, EyePosition::LEFT, this->first_eye_threshold);
		bool right = cutEyeRegion(frame, right_eye, EyePosition::RIGHT, this->first_eye_threshold);

		return std::make_pair(left ? left_eye : cv::Mat(), right ? right_eye : cv::Mat());
	};
	return new BoreTrainer(wrapper, detection, eye_separation, 20);
}


PupilGlintCombiInstance GoalkeeperAnalysis::detectRaw(const cv::Mat& frame)
{
	return detectGlintAndPupilNEW(frame, first_eye_threshold, pupil_detection.get() );
}

PupilGlintCombiInstance GoalkeeperAnalysis::detectRaw(const SparseInputImage& frame)
{
	return detectGlintAndPupilNEWSparse(frame, first_eye_threshold, pupil_detection.get());
}


ScreenspaceGazeResult GoalkeeperAnalysis::gazeEstimationFunction(const DetectedFeatures& instance)
{
	auto result_right = singleRightGazeEstimationFunction(instance);
	auto result_left = singleLeftGazeEstimationFunction(instance);

	if (result_right.valid && result_left.valid) 
	{
		return ScreenspaceGazeResult(0.5 * (result_right.position + result_left.position));
	}
	else if(result_right.valid)
	{
		return result_right;
	}
	else if(result_left.valid)
	{
		return result_left;
	}
	return ScreenspaceGazeResult();
}

ScreenspaceGazeResult GoalkeeperAnalysis::singleLeftGazeEstimationFunction(const DetectedFeatures& instance)
{
	if (!instance.left_eye.allValid())
		return ScreenspaceGazeResult();

	PupilCenterGlintInputs inputs; // only one camera here.

	inputs.data = { static_cast<PupilCenterGlintInput>(instance.left_eye) };


	auto pog = this->gaze_estimation_left->estimate(inputs, this->setup_parameters_left);
	Vec3 poi_world = calculatePointOfInterest(pog.center_of_cornea, pog.visual_axis, z_shift);
	poi_world -= wcs_offset;
	Vec2 poi_screen = estimateScreenPoint(poi_world, screen_pixel_size_cm[0], screen_pixel_size_cm[1]);

	return ScreenspaceGazeResult(cv::Point2f(poi_screen[0], poi_screen[1]));
}

ScreenspaceGazeResult GoalkeeperAnalysis::singleRightGazeEstimationFunction(const DetectedFeatures& instance)
{
	if (!instance.right_eye.allValid())
		return ScreenspaceGazeResult();

	PupilCenterGlintInputs inputs; // only one camera here.

	inputs.data = { static_cast<PupilCenterGlintInput>(instance.right_eye) };


	auto pog = this->gaze_estimation_right->estimate(inputs, this->setup_parameters_right);

	Vec3 distance_to_camera = pog.center_of_cornea - setup_parameters_right.cameras[0].position;
	
	//gProfilingMonitor.addTiming("Estimated Distance to cam:", ProfilingDatapoint(length(distance_to_camera)));
	//gProfilingMonitor.addTiming("re_subject_dist", ProfilingDatapoint(length(distance_to_camera)));
	//gProfilingMonitor.addTiming("glint0_x", ProfilingDatapoint(instance.right_eye.glint(0).x));
	//gProfilingMonitor.addTiming("glint0_y", ProfilingDatapoint(instance.right_eye.glint(0).y));
	//setup_parameters_right.distance_to_camera_estimate = length(distance_to_camera);

	Vec3 poi_world = calculatePointOfInterest(pog.center_of_cornea, pog.visual_axis, z_shift);

	poi_world -= wcs_offset;
	Vec2 poi_screen = estimateScreenPoint(poi_world, screen_pixel_size_cm[0], screen_pixel_size_cm[1]);

	return ScreenspaceGazeResult(cv::Point2f(poi_screen[0], poi_screen[1]));
}

class StopAllOngoingEvent : public QEvent
{
public:
	static const QEvent::Type my_type = static_cast<QEvent::Type>(QEvent::User + 15);
	explicit StopAllOngoingEvent() : QEvent(my_type) {}
};




static void onGammaChange(int v, void* ptr);
static void onGainChange(int v, void* ptr);
static void onHWGainChange(int v, void* ptr);


GoalkeeperAnalysis::GoalkeeperAnalysis(QWidget* parent):
	QMainWindow(parent),
	ui(new Ui::GoalkeeperAnalysisClass),
	current_mode(OperationMode::NONE),
	first_eye_threshold(initial_first_eye_threshold),
	user_folder("default_output"),
	calibrated_for_current(false),
	perfStatsRefreshTimer(new QTimer(this)),
	filter_ri_pupil(noise_reduction_filter_length),
	filter_ri_coc(noise_reduction_filter_length),
	filter_le_pupil(noise_reduction_filter_length),
	filter_le_coc(noise_reduction_filter_length),
	gaze_maFilter(MAFilter(30, true, 5)),	
	pupil_right_maFilter(MAFilter(20, true, 1)),
	pupil_left_maFilter(MAFilter(20, true, 1))		// 3 oder 4 sieht gut aus mit 20 samples besser als mit 10
	
{
	_STILLIMAGE_STIMULUS = 0;

	 currentPupilRight = { -1.0f, -1.0f };
	 currentPupilLeft = { -1.0f, -1.0f };
	 currentGlint1 = { -1.0f, -1.0f };
	 currentGlint2 = { -1.0f, -1.0f };
	 currentGlint3 = { -1.0f, -1.0f };

	 old_gaze = { -1.0f,-1.0f };


	ui->setupUi(this);

	pupil_detection.reset(new BoreWrapper());
	
	ELSE::m_load_1("ELSE_trainingFiles\\detector.txt");
	ELSE::init_mats_1();
	ELSE::cmp_indexes_1();

	ELSE::m_load_2("ELSE_trainingFiles\\detector.txt");
	ELSE::init_mats_2();
	ELSE::cmp_indexes_2();
	
	ensureDirectoryExists(user_folder);

	connect(ui->startCaptureButton, SIGNAL(clicked()), this, SLOT(startCaptureSession()));
	connect(ui->startCalibrationButton, SIGNAL(clicked()), this, SLOT(startCalibration()));
	connect(ui->calibrationEvaluation, SIGNAL(clicked()), this, SLOT(startEvaluatingCalibrationAccuracy()));
	connect(ui->stopCaptureButton, SIGNAL(clicked()), this, SLOT(stopAllOngoing()));
	connect(ui->saveEyeImages, SIGNAL(clicked()), this, SLOT(startSavingEyevideo()));
	connect(ui->showPlaybackButton, SIGNAL(clicked()), this, SLOT(startPlaybackSession()));
	connect(ui->getTestGaze, SIGNAL(clicked()), this, SLOT(startLiveTesting()));

	//connect(ui->captureFullimage, SIGNAL(clicked()), this, SLOT(captureFullimage()));
	//connect(ui->getAccuracy, SIGNAL(clicked()), this, SLOT(calculateAccuracy()));
	

	// show calibration detection results buttons
	//connect(ui->rb_POLY_1_X_Y_XY_XX_YY_XYY_YXX_XXX_YYY, SIGNAL(clicked()), this,
	//	SLOT(rb_POLY_1_X_Y_XY_XX_YY_XYY_YXX_XXX_YYY())); // show test image and print live captured gaze on it
	
	setupDefaultSetup();


	cv::namedWindow(control_window_id, CV_WINDOW_NORMAL);
	cv::createTrackbar("1st Thresh:", control_window_id, &first_eye_threshold, 255);
	cv::createTrackbar("Gain", control_window_id, &gain_slider_value, 255, onGainChange, this);
	cv::createTrackbar("Gamma", control_window_id, &gamma_slider_value, 220, onGammaChange, this);
	cv::createTrackbar("HW_Gain", control_window_id, &hw_gain_slider_value, 220, onHWGainChange, this);


	camera.reset(new uEyeCamera());

	camera->setGamma(gamma_slider_value);
	camera->setGain(gain_slider_value);
	camera->setGainBoost();
	camera->setHWGain(hw_gain_slider_value);
	camera->getSensorInformation();
	camera->activateInternalMemory();
	camera->getCameraInformation();

	// set AOI for speed up. not all formats are supported as stated in the API.
	camera->setAOI(150, 10, 640, 360);

	camera->InitMemory();
	camera->setPixelclock();
	camera->setFrameRate(target_camera_fps);
	camera->setExposure(target_exposure);
	camera->setFlash();
	

	
	gaze_estimation_left.reset(new gazeestimation::OneCamSphericalGE(use_chen_noise_reduction));
	gaze_estimation_right.reset(new gazeestimation::OneCamSphericalGE(use_chen_noise_reduction));

	if(use_noise_reduction_filter){
		gaze_estimation_left->setCorneaCenterFilter(std::bind(&Vec3MedianFilter::newSample, &filter_le_coc, std::placeholders::_1));
		gaze_estimation_left->setPupilCenterFilter(std::bind(&Vec3MedianFilter::newSample, &filter_le_pupil, std::placeholders::_1));
		gaze_estimation_right->setCorneaCenterFilter(std::bind(&Vec3MedianFilter::newSample, &filter_ri_coc, std::placeholders::_1));
		gaze_estimation_right->setPupilCenterFilter(std::bind(&Vec3MedianFilter::newSample, &filter_ri_pupil, std::placeholders::_1));
	}
	
	QObject::connect(perfStatsRefreshTimer, SIGNAL(timeout()), this, SLOT(refreshProfilingInfo()));
	perfStatsRefreshTimer->start(1000);

	QInputDialog* choose_directory = new QInputDialog(this);
	choose_directory->setInputMode(QInputDialog::InputMode::TextInput);
	choose_directory->setLabelText("Name: ");
	connect(choose_directory, SIGNAL(textValueSelected(const QString &)), this, SLOT(userDirectoryChanged(const QString&)));
	choose_directory->show();

	lastFrameReceived = false;
	stimulusStack = new std::vector<cv::Mat*>();

	// ==================================
	pupilR_maFilter = MAFilter(20, true, 1);
	pupilL_maFilter = MAFilter(20, true, 1);
	// ==================================


}

GoalkeeperAnalysis::~GoalkeeperAnalysis()
{
	stopAllOngoing();
	
	camera->exit();
	camera->closeCamera();
	camera.reset(nullptr);

}

void GoalkeeperAnalysis::receiveFrame(const cv::Mat& original_frame)
{
	const FrameTimestamp origin_time = getCurrentTime();
	frame_counter++;

	gProfilingMonitor.addTiming("Receive Frame Processing", ProfilingSection(), true);
	ProfilingSection ps;
	ProfilingSection ps_without_display;

	cv::Mat frame;
	if(original_frame.channels() > 1)
	{
		cvtColor(original_frame, frame, CV_BGR2GRAY);
	}else
	{
		frame = original_frame;
	}


	if (_ONLINE && lastFrameReceived && current_mode == OperationMode::SAVING) {

		ScreenspaceGazeResult gaze = getGazeAndWriteToFile(frame, eyeFile,
			std::bind(&GoalkeeperAnalysis::gazeEstimationFunction, this, std::placeholders::_1), pupil_detection.get(),
			first_eye_threshold);
		PupilGlintCombiInstance features = detectRaw(frame);

		DetectedFeatures detected_features(features);
		PupilGlintCombiInstance filteredFeatures = use_lowpass_filter ? filterFeatures(features) : features;
		auto gaze2 = gazeEstimationFunction(DetectedFeatures(filteredFeatures));


		auto filteredGaze = lowPassGaze(gaze2);
		cv::Mat temp;
		if (_STILLIMAGE_STIMULUS) {
			int width = (int)GetSystemMetrics(SM_CXSCREEN);
			int height = (int)GetSystemMetrics(SM_CYSCREEN);
			cv::resize(*stillImage, temp, cv::Size(width, height));
		}	
		else
			lastFrame.copyTo(temp);

		cv::circle(temp, filteredGaze.position, 10, cv::Scalar(0, 255, 0), CV_FILLED);
		(*online_writer) << temp;

	}

	if(current_mode == OperationMode::SAVING && video_recorder != nullptr)
	{
		QueuedFrame queued_frame;
		queued_frame.frame = frame;
		queued_frame.origin_time = origin_time;
		video_recorder->enqueueFrame(queued_frame);
	}

	if(current_mode == OperationMode::EYEIMAGES && video_recorder != nullptr)
	{
		cv::Mat eye_image;
		if(cutEyeRegion(frame, eye_image, EyePosition::RIGHT, first_eye_threshold))
		{
			QueuedFrame queued_frame;
			queued_frame.frame = eye_image;
			queued_frame.origin_time = origin_time;
			video_recorder->enqueueFrame(queued_frame);
		}
	}

	if (current_mode == OperationMode::SAVING || current_mode == OperationMode::EYEIMAGES)
		return;
	
	if (new_frame_processing_lockout)
		return;

	PupilGlintCombiInstance features = detectRaw(frame);

	DetectedFeatures detected_features(features);
	PupilGlintCombiInstance filteredFeatures = use_lowpass_filter? filterFeatures(features) : features;
	


	if(calibration_dialog != nullptr)
	{
		/*cv::Mat eye_image;
		if (cutEyeRegion(frame, eye_image, EyePosition::RIGHT, first_eye_threshold))
		{
			QueuedFrame queued_frame;
			queued_frame.frame = eye_image;
			queued_frame.origin_time = origin_time;
			calibration_dialog->receiveRawImage(queued_frame);
		}*/
		if(calibration_dialog->needsDetectedFeatures())
		{
			calibration_dialog->receiveEyeFeatures(filteredFeatures);
		}
		else
		{
			QueuedFrame queued_frame;
			queued_frame.frame = frame;
			queued_frame.origin_time = origin_time;
			calibration_dialog->receiveRawImage(queued_frame);
		}
	}

	gProfilingMonitor.addTiming("receiveFrame - without display", ps_without_display);

	if(display_found_features)
	{
		//input_features_display->imageAvailable(frame, DetectedFeatures(features));
		input_features_display->imageAvailable(frame, DetectedFeatures(filteredFeatures));
	}


	if(live_display != nullptr)
	{
		
		//auto gaze = gazeEstimationFunction(DetectedFeatures(features));
		auto gaze = gazeEstimationFunction(DetectedFeatures(filteredFeatures));

		auto filteredGaze = lowPassGaze(gaze);

		if (filteredGaze.valid)
			live_display->setGaze(QPoint(-1, -1), QPoint(-1, -1), QPoint(filteredGaze.position.x, filteredGaze.position.y));
		else
			live_display->setGaze(QPoint(-1, -1), QPoint(-1, -1), QPoint(-1, -1));
	}
	
	gProfilingMonitor.addTiming("receiveFrame", ps);
}



PupilGlintCombiInstance GoalkeeperAnalysis::filterFeatures(PupilGlintCombiInstance detectedFeatures)
{
	PupilGlintCombiInstance out;
	out = detectedFeatures;
		
	// right Pupil
	if (detectedFeatures.pupilRight.x > 0 && detectedFeatures.pupilRight.y > 0)
	{
		out.pupilRight = pupil_right_maFilter.movingAveragesFilter(detectedFeatures.pupilRight);
	}
	else
	{
		out.detectedRight = false;
	}
	

	if (detectedFeatures.glintRight_1.x > 0 && detectedFeatures.glintRight_1.y > 0)
	{
		KALMAN::GlintFilter::correct(KALMAN::GlintFilter::RIGHT_0, detectedFeatures.glintRight_1.x, detectedFeatures.glintRight_1.y);
		out.glintRight_1 = detectedFeatures.glintRight_1;

	}
	

	if (detectedFeatures.glintRight_2.x > 0 && detectedFeatures.glintRight_2.y > 0)
	{
		KALMAN::GlintFilter::correct(KALMAN::GlintFilter::RIGHT_1, detectedFeatures.glintRight_2.x, detectedFeatures.glintRight_2.y);
		out.glintRight_2 = detectedFeatures.glintRight_2;
		
	}


	if (detectedFeatures.glintRight_3.x > 0 && detectedFeatures.glintRight_3.y > 0)
	{
		KALMAN::GlintFilter::correct(KALMAN::GlintFilter::RIGHT_2, detectedFeatures.glintRight_3.x, detectedFeatures.glintRight_3.y);
		out.glintRight_3 = detectedFeatures.glintRight_3;
		
	}



	// left Pupil
	if (detectedFeatures.pupilLeft.x > 0 && detectedFeatures.pupilLeft.y > 0)
	{
		out.pupilLeft = pupil_left_maFilter.movingAveragesFilter(detectedFeatures.pupilLeft);
	}
	else
	{
		out.detectedLeft = false;
	}


	if (detectedFeatures.glintLeft_1.x > 0 && detectedFeatures.glintLeft_1.y > 0)
	{
		KALMAN::GlintFilter::correct(KALMAN::GlintFilter::LEFT_0, detectedFeatures.glintLeft_1.x, detectedFeatures.glintLeft_1.y);
		out.glintLeft_1 = detectedFeatures.glintLeft_1;

	}


	if (detectedFeatures.glintLeft_2.x > 0 && detectedFeatures.glintLeft_2.y > 0)
	{
		KALMAN::GlintFilter::correct(KALMAN::GlintFilter::LEFT_1, detectedFeatures.glintLeft_2.x, detectedFeatures.glintLeft_2.y);
		out.glintLeft_2 = detectedFeatures.glintLeft_2;

	}


	if (detectedFeatures.glintLeft_3.x > 0 && detectedFeatures.glintLeft_3.y > 0)
	{
		KALMAN::GlintFilter::correct(KALMAN::GlintFilter::LEFT_2, detectedFeatures.glintLeft_3.x, detectedFeatures.glintLeft_3.y);
		out.glintLeft_3 = detectedFeatures.glintLeft_3;

	}



	return out;
}

ScreenspaceGazeResult GoalkeeperAnalysis::lowPassGaze(ScreenspaceGazeResult gaze)
{
	cv::Point2f filteredGaze = gaze.position;
	
	if (gaze.position.x > 0 && gaze.position.y > 0)
	{
		//out = gaze_maFilter.movingAveragesFilter(gaze.position);
			KALMAN::GlintFilter::correct(KALMAN::GlintFilter::GAZE, filteredGaze.x, filteredGaze.y, 100.0,100.0, 40.0);
	}
	
	return ScreenspaceGazeResult(filteredGaze);
	
			
}

ScreenspaceGazeResult GoalkeeperAnalysis::getGazeAndWriteToFile(cv::Mat frame,
	std::ofstream* output_file,
	DetectionToGazeEstimation estimation,
	PupilDetection* pupil_detection,
	int first_eye_threshold)
{
	PupilGlintCombiInstance pgci = detectGlintAndPupilNEW(frame, first_eye_threshold, pupil_detection);

	PupilGlintCombiInstance ergebnis = pgci;
	// right Pupil
	if (pgci.pupilRight.x > 0 && pgci.pupilRight.y > 0)
	{
		ergebnis.pupilRight = pupilR_maFilter.movingAveragesFilter(pgci.pupilRight);
	}


	if (ergebnis.glintRight_1.x > 0 && ergebnis.glintRight_1.y)
	{
		KALMAN::GlintFilter::correct(KALMAN::GlintFilter::RIGHT_0, ergebnis.glintRight_1.x, ergebnis.glintRight_1.y);
	}


	if (ergebnis.glintRight_2.x > 0 && ergebnis.glintRight_2.y > 0)
	{
		KALMAN::GlintFilter::correct(KALMAN::GlintFilter::RIGHT_1, ergebnis.glintRight_2.x, ergebnis.glintRight_2.y);

	}


	if (ergebnis.glintRight_3.x > 0 && ergebnis.glintRight_3.y > 0)
	{
		KALMAN::GlintFilter::correct(KALMAN::GlintFilter::RIGHT_2, ergebnis.glintRight_3.x, ergebnis.glintRight_3.y);

	}



	// left Pupil
	if (pgci.pupilLeft.x > 0 && pgci.pupilLeft.y > 0)
	{
		ergebnis.pupilLeft = pupilL_maFilter.movingAveragesFilter(pgci.pupilLeft);
		//qDebug() << "New Pupilleft: " << ergebnis.pupilLeft.x << " " << ergebnis.pupilLeft.y;
	}


	if (ergebnis.glintLeft_1.x > 0 && ergebnis.glintLeft_1.y > 0)
	{
		KALMAN::GlintFilter::correct(KALMAN::GlintFilter::LEFT_0, ergebnis.glintLeft_1.x, ergebnis.glintLeft_1.y);

	}


	if (ergebnis.glintLeft_2.x > 0 && ergebnis.glintLeft_2.y > 0)
	{
		KALMAN::GlintFilter::correct(KALMAN::GlintFilter::LEFT_1, ergebnis.glintLeft_2.x, ergebnis.glintLeft_2.y);

	}


	if (ergebnis.glintLeft_3.x > 0 && ergebnis.glintLeft_3.y > 0)
	{
		KALMAN::GlintFilter::correct(KALMAN::GlintFilter::LEFT_2, ergebnis.glintLeft_3.x, ergebnis.glintLeft_3.y);

	}

	// unfiltered
	//auto gaze = estimation(DetectedFeatures(pgci));

	// filtered
	auto gaze = estimation(DetectedFeatures(ergebnis));

	//qDebug() << "FilteredGaze: " << gaze.position.x << " " << gaze.position.y;
	// ================================
	//auto gaze = estimation(DetectedFeatures(pgci));
	// ================================


	// ================================
	// GAZE: Kalman Filter 
	cv::Point2f gazePostition = gaze.position;
	// TO-DO KALMAN::GlintFilter::correct(KALMAN::GlintFilter::GAZE, gazePostition.x, gazePostition.y, 100.0, 100.0, 40.0);

	ScreenspaceGazeResult filteredGaze(gazePostition);

	//qDebug() << "Gaze :" << gaze.position.x << "/" << gaze.position.y;

	// save infos to file
	double time = getCurrentTime();

	(*output_file) << time << " " << pgci.pupilRight << " " << pgci.pupilLeft << "  " << pgci.glintRight_1 << " "
		<< pgci.glintRight_2 << " " << pgci.glintRight_3 << " " << pgci.glintLeft_1 <<
		" " << pgci.glintLeft_2 << " " << pgci.glintLeft_3 << gaze.position.x << " " << gaze.position.y << std::endl;
	

	return gaze;
}


void GoalkeeperAnalysis::startCaptureSession()
{
	display_found_features = false;		// speed up

	boost::filesystem::path stimulus_target_path(user_folder + "/stimulus.avi");
	if (!boost::filesystem::exists(stimulus_target_path) && !_STILLIMAGE_STIMULUS)
	{
		QMessageBox box;
		box.setText("You need to set a stimulus file first.");
		box.setStandardButtons(QMessageBox::Cancel);
		box.exec();
		return;
	}

	if (!calibrated_for_current)
	{
		QMessageBox box;
		box.setText("Warning: There has been no calibration for the current user.");
		box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		box.setDefaultButton(QMessageBox::Cancel);
		int res = box.exec();
		if (res == QMessageBox::Cancel)
			return;
	}

	current_mode = OperationMode::SAVING;

	setupCapture(OperationMode::SAVING);

	std::string filename = user_folder + "/EyeVideo_" + std::to_string(getCurrentTime()) + ".avi";
	auto recorder = new VideoWriterThread(filename, lossless_video_codec, video_frame_size, video_framerate);
	if (!recorder->start())
	{
		qWarning() << "Couldn't start VideoWriter.";
		return;
	}


	video_recorder.reset(recorder);
	capture->startCapture();

	
	if (_STILLIMAGE_STIMULUS) {
		stimulusEndHotkey = new QAction(this);
		stimulusEndHotkey->setShortcut(Qt::Key_Escape);
		connect(stimulusEndHotkey, SIGNAL(triggered()), this, SLOT(GoalkeeperAnalysis::stimulusPlaybackEnded()));
		this->addAction(stimulusEndHotkey);

		stimulus_meta = {};
		stimulus_meta.start_time = getCurrentTime();
		cv::namedWindow("Stimulus Display", cv::WND_PROP_FULLSCREEN);
		cv::setWindowProperty("Stimulus Display", cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
		for (int i = 0; i < stimulusStack->size(); i++) {
			stillImage = stimulusStack->at(i);
			cv::imshow("Stimulus Display", *stillImage);
			cv::waitKey(0);
		}
		lastFrameReceived = false; // to not write more to the video file
		qDebug() << "Pressed key, stimulus ended";
		stimulusPlaybackEnded();
	}
	else {
		auto full_path = boost::filesystem::canonical(stimulus_target_path);
		auto stimulus_source = new VideoFilePollingThread(full_path.string(), false, [](const cv::Mat&) {});
		cv::namedWindow(stimulus_window_id, cv::WINDOW_FULLSCREEN);
		//cv::resizeWindow(stimulus_window_id, stimulus_source->videoSize().width, stimulus_source->videoSize().height);
		//cv::moveWindow(stimulus_window_id, 0, 0);
		stimulus_position = cv::Rect(0, 0, stimulus_source->videoSize().width, stimulus_source->videoSize().height);
		//stimulus_playback_thread.reset(stimulus_source);
		stimulus_source->setExtendedFrameCallback(std::bind(&GoalkeeperAnalysis::showStimulusFrame, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		stimulus_source->setEndOfFileCallback(std::bind(&GoalkeeperAnalysis::stimulusPlaybackEnded, this));

		stimulus_playback.reset(new VideoCaptureSession(stimulus_source));
		stimulus_meta = {};
		stimulus_meta.start_time = getCurrentTime();
		stimulus_playback->startCapture();
	}
}


void GoalkeeperAnalysis::showStimulusFrame(const cv::Mat& frame, unsigned int frame_index, double time)
{
	stimulus_meta.frame2time.emplace(frame_index, time);
	cv::imshow(stimulus_window_id, frame);
	frame.copyTo(lastFrame);
	lastFrameReceived = true;
	cv::waitKey(1);

}

void GoalkeeperAnalysis::stimulusPlaybackEnded()
{
	stimulus_meta.end_time = getCurrentTime();

	// terminate the capture and wait until the recording is finished being written to before operating on the data
	capture.reset(nullptr);
	video_recorder->stop();
	if (_STILLIMAGE_STIMULUS) {

	}
	if (_ONLINE) {
		online_writer->release();

		video_recorder.reset(nullptr);

		const auto stimulus_meta_filename = user_folder + "/stimulus.txt";
		if (!writeUserVideoFile(stimulus_meta_filename, stimulus_meta))
		{
			qWarning() << "Couldn't write " << stimulus_meta_filename.c_str();
		}

		cv::destroyAllWindows();

		QCoreApplication::postEvent(this, new StopAllOngoingEvent(), Qt::NormalEventPriority);
			   
		// finish file then
		(*eyeFile)<< "End: " << stimulus_meta.end_time << std::endl;

		qDebug() << "Eyevideo END-Timestamp: " << stimulus_meta.end_time;

		// close file
		if (eyeFile->is_open())
		{
			eyeFile->close();
		}
		return;
	}

	const std::string eye_video_filename = video_recorder->filename();
	VideoFrame2Timestamp eye_video_meta = video_recorder->videoInfo();

	video_recorder.reset(nullptr);

	const auto stimulus_meta_filename = user_folder + "/stimulus.txt";
	if (!writeUserVideoFile(stimulus_meta_filename, stimulus_meta))
	{
		qWarning() << "Couldn't write " << stimulus_meta_filename.c_str();
	}


	cv::destroyAllWindows();

	auto gaze_meta = estimateAndSaveEyeVideoFile(eye_video_filename, user_folder + "/eyes.txt", eye_video_meta, 
		std::bind(&GoalkeeperAnalysis::gazeEstimationFunction, this, std::placeholders::_1), pupil_detection.get(),
		first_eye_threshold);

	drawGaze(user_folder + "/stimulus.avi", user_folder + "/output.avi", stimulus_meta, eye_video_meta, gaze_meta, stimulus_position);

	QCoreApplication::postEvent(this, new StopAllOngoingEvent(), Qt::NormalEventPriority);
}

void GoalkeeperAnalysis::calibrationFinished()
{
	new_frame_processing_lockout = true;
	// first close the video source, because we can't simultaneously use
	// the pupil detection for new inputs and recalibrate it.
	capture.reset(nullptr);

	CalibrationData result = calibration_dialog->getCalibratedData();
	stopAllOngoing();
	calibrateFrom(result);
	new_frame_processing_lockout = false;

	//TODO: Make this conditional on how good the calibration result actually was.
	calibrated_for_current = true;
}

void GoalkeeperAnalysis::calibrationAccuracyFinished()
{
	CalibrationData data = calibration_dialog->getCalibratedData();
	stopAllOngoing();

	if (gaze_estimation_right != nullptr)
	{
		auto evaluation = evaluateAccuracy(data, screen_pixel_size_cm,
			std::bind(&GoalkeeperAnalysis::singleRightGazeEstimationFunction, this, std::placeholders::_1),
			std::bind(&GoalkeeperAnalysis::resetGazeEstimationFilters, this),
			CalibrationEvaluationMode::RIGHT_ONLY);

		evaluation.writeRawData(user_folder + "/evaluationRawdata.txt");
		evaluation.writeEstimationsToFile(user_folder + "/evaluationEstimations.txt");

		evaluation.writeToFile(user_folder + "/calibrationResultsRightAccuracy.txt");
		evaluation.writeImage(user_folder + "/calibrationResultsRightAccuracy.png");
		evaluation.writeInputEvluationImage(user_folder + "/calibrationResultsInputEvalAccuracy.png", user_folder + "/calibrationResultsInputEvalAccuracyGlints.png");
	}
	
	if (gaze_estimation_left != nullptr)
	{
		auto evaluation = evaluateAccuracy(data, screen_pixel_size_cm,
			std::bind(&GoalkeeperAnalysis::singleLeftGazeEstimationFunction, this, std::placeholders::_1),
			std::bind(&GoalkeeperAnalysis::resetGazeEstimationFilters, this), CalibrationEvaluationMode::LEFT_ONLY);
		evaluation.writeRawData(user_folder + "/evaluationRawdata.txt");
		evaluation.writeEstimationsToFile(user_folder + "/evaluationEstimations.txt");

		evaluation.writeToFile(user_folder + "/calibrationResultsLeftAccuracy.txt");
		evaluation.writeImage(user_folder + "/calibrationResultsLeftAccuracy.png");
	}

	auto evaluation = evaluateAccuracy(data, screen_pixel_size_cm,
		std::bind(&GoalkeeperAnalysis::gazeEstimationFunction, this, std::placeholders::_1),
		std::bind(&GoalkeeperAnalysis::resetGazeEstimationFilters, this), CalibrationEvaluationMode::BOTH);
	evaluation.writeRawData(user_folder + "/evaluationRawdata.txt");
	evaluation.writeToFile(user_folder + "/calibrationResultsCombinedAccuracy.txt");
	evaluation.writeImage(user_folder + "/calibrationResultsCombinedAccuracy.png");
	evaluation.writeFeatureDetectionRate(user_folder + "/evaluationDetectionRates.txt");
}


void GoalkeeperAnalysis::liveGazeDisplayClosed()
{
	if(live_display != nullptr)
		QTimer::singleShot(40, this, SLOT(stopAllOngoing()));
}

void GoalkeeperAnalysis::startCalibration()
{
	// CHANGED AUG
	display_found_features = false;

	current_mode = OperationMode::CALIBRATION;

	// setup capture
	setupCapture(OperationMode::CALIBRATION);
	setupDefaultSetup();

	// setup calibration dialog
	auto screen_parameters = QApplication::desktop()->screenGeometry();
	auto calibration_points = FreeformPatternGenerator(screen_parameters.width(), screen_parameters.height(), 9).calibrationPoints();

	capture->startCapture();

	calibration_trainer.reset(createTrainer(pupil_detection.get()));
	calibration_dialog.reset(new CalibrationDialog(calibration_points, 
		std::make_pair(screen_parameters.width(), screen_parameters.height()), 
		calibration_trainer.get(), 
		this));

	connect(calibration_dialog.get(), SIGNAL(calibrationFinished()), this, SLOT(calibrationFinished()), Qt::QueuedConnection);
	calibration_dialog->showFullScreen();
}

void GoalkeeperAnalysis::startSavingEyevideo()
{
	current_mode = OperationMode::EYEIMAGES;

	setupCapture(OperationMode::EYEIMAGES);
	video_frame_size = cv::Size(100, 100); //TODO: Remove hardcoded ROI for eye region.

	auto recorder = new VideoWriterThread("test_eyevideo.avi", lossless_video_codec, video_frame_size, video_framerate);
	if (!recorder->start())
	{
		qWarning() << "Couldn't start VideoWriter.";
		return;
	}

	video_recorder.reset(recorder);
	capture->startCapture();
}

void GoalkeeperAnalysis::startEvaluatingCalibrationAccuracy()
{
	display_found_features = false;

	if(!calibrated_for_current)
	{
		QMessageBox box;
		box.setText("Warning: There has not been any calibration for this user yet.");
		box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		box.setDefaultButton(QMessageBox::Cancel);
		int res = box.exec();
		if (res == QMessageBox::Cancel)
		{
			return;
		}		
	}

	current_mode = OperationMode::CALIBRATION_ACCURACY;

	// setup capture
	setupCapture(OperationMode::CALIBRATION);

	// setup calibration dialog
	auto screen_parameters = QApplication::desktop()->screenGeometry();;
	auto calibration_points = FreeformPatternGenerator(screen_parameters.width(), screen_parameters.height(), 9).calibrationPoints();

	capture->startCapture();

	calibration_trainer.reset(new NoTraining());
	calibration_dialog.reset(new CalibrationDialog(calibration_points, 
		std::make_pair(screen_parameters.width(), screen_parameters.height()),
		calibration_trainer.get(), 
		this));

	connect(calibration_dialog.get(), SIGNAL(calibrationFinished()), this, SLOT(calibrationAccuracyFinished()), Qt::QueuedConnection);
	calibration_dialog->showFullScreen();
}

void GoalkeeperAnalysis::startPlaybackSession()
{
	current_mode = OperationMode::LIVE_GAZE;

	live_display.reset(new LiveGazeDisplay(60, nullptr));	// 60
	live_display->show();
	connect(live_display.get(), SIGNAL(ended()), this, SLOT(liveGazeDisplayClosed() ));

	setupCapture(OperationMode::LIVE_GAZE);

	capture->startCapture();
}

void GoalkeeperAnalysis::startLiveTesting()
{
	display_found_features = true;		// deactivated for speed up

	current_mode = OperationMode::LIVE_TESTING;
	setupCapture(OperationMode::LIVE_TESTING);

	capture->startCapture();	
}

void GoalkeeperAnalysis::startCaptureFullImage()
{
	QMessageBox box;
	box.setText("This currently doesn't do anything.");
	box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	box.setDefaultButton(QMessageBox::Cancel);
	int res = box.exec();
	return;	
}

void GoalkeeperAnalysis::stopAllOngoing()
{
	capture.reset(nullptr);
	video_recorder.reset(nullptr);
	stimulus_playback.reset(nullptr);
	

	QCoreApplication::processEvents(QEventLoop::AllEvents);
	if(calibration_dialog != nullptr){
		calibration_dialog->close();
		calibration_dialog.reset(nullptr);
	}

	if(live_display != nullptr)
	{
		live_display->close();
		live_display.reset(nullptr);
	}

	if(input_features_display != nullptr)
	{
		input_features_display->close();
		input_features_display.reset(nullptr);
	}

	calibration_trainer.reset(nullptr);

	cv::destroyAllWindows();
	current_mode = OperationMode::NONE;
}

void GoalkeeperAnalysis::refreshProfilingInfo()
{
	QString report = gProfilingMonitor.generateReport();
	QString header("Profiling [over last ");
	header = header.append(QString::number(profilingMonitorMaxObjectsRetained)).append(" samples]\n");

	ui->profilingLabel->setText(header+report);
}

void GoalkeeperAnalysis::resetGazeEstimationFilters()
{
	filter_ri_pupil.reset();
	filter_le_pupil.reset();
	filter_ri_coc.reset();
	filter_le_coc.reset();
}


void appendCalibrationInfo(const std::string& filename, const EyeAndCameraParameters& setup)
{
	std::ofstream file(filename, std::ios_base::app);
	file << "Calibrated variables: \n";

	file << "Calibration result: " << std::endl;
	file << "Alpha: " << setup.alpha << " (" << rad_to_deg(setup.alpha) << ")" << std::endl;
	file << "Beta: " << setup.beta << " (" << rad_to_deg(setup.beta) << ")" << std::endl;
	file << "R: " << setup.R << std::endl;
	file << "K: " << setup.K << std::endl;
	file << "CamAy: " << setup.cameras[0].camera_angle_y() << " (" << rad_to_deg(setup.cameras[0].camera_angle_y()) << ")" << std::endl;
	file << "CamAz: " << setup.cameras[0].camera_angle_z() << " (" << rad_to_deg(setup.cameras[0].camera_angle_z()) << ")" << std::endl;
	file << std::endl;
	file.close();
}

void GoalkeeperAnalysis::calibrateFrom(const CalibrationData& data)
{
	// NOTE: Filters must be disabled during calibration, because they will interfere with it.
	if (use_noise_reduction_filter) {
		gaze_estimation_left->setCorneaCenterFilter(gazeestimation::OneCamSphericalGE::Vec3Filter());
		gaze_estimation_left->setPupilCenterFilter(gazeestimation::OneCamSphericalGE::Vec3Filter());
		gaze_estimation_right->setCorneaCenterFilter(gazeestimation::OneCamSphericalGE::Vec3Filter());
		gaze_estimation_right->setPupilCenterFilter(gazeestimation::OneCamSphericalGE::Vec3Filter());
	}

	if(gaze_estimation_right != nullptr){
		calibrateGazeEstimationFrom(setup_parameters_right, data, gaze_estimation_right.get(), EyePosition::RIGHT, screen_pixel_size_cm, wcs_offset, z_shift);

		auto evaluation = evaluateAccuracy(data, screen_pixel_size_cm,
			std::bind(&GoalkeeperAnalysis::singleRightGazeEstimationFunction, this, std::placeholders::_1),
			std::bind(&GoalkeeperAnalysis::resetGazeEstimationFilters, this), CalibrationEvaluationMode::RIGHT_ONLY);

		evaluation.writeRawData(user_folder + "/calibrationRawData.txt");
		evaluation.writeEstimationsToFile(user_folder + "/calibrationEstimations.txt");

		evaluation.writeToFile(user_folder + "/calibrationResultsRight.txt");
		evaluation.writeImage(user_folder + "/calibrationResultsRight.png");
		evaluation.writeInputEvluationImage(user_folder + "/calibrationResultsInputEval.png", user_folder + "/calibrationResultsInputEvalGlints.png");
		appendCalibrationInfo(user_folder + "/calibrationResultsRight.txt", setup_parameters_right);
	}

	if(gaze_estimation_left != nullptr){
		calibrateGazeEstimationFrom(setup_parameters_left, data, gaze_estimation_left.get(), EyePosition::LEFT, screen_pixel_size_cm, wcs_offset, z_shift);

		auto evaluation = evaluateAccuracy(data, screen_pixel_size_cm,
			std::bind(&GoalkeeperAnalysis::singleLeftGazeEstimationFunction, this, std::placeholders::_1),
			std::bind(&GoalkeeperAnalysis::resetGazeEstimationFilters, this), CalibrationEvaluationMode::LEFT_ONLY);
		evaluation.writeToFile(user_folder + "/calibrationResultsLeft.txt");
		evaluation.writeImage(user_folder + "/calibrationResultsLeft.png");
		appendCalibrationInfo(user_folder + "/calibrationResultsLeft.txt", setup_parameters_left);
	}

	auto evaluation = evaluateAccuracy(data, screen_pixel_size_cm,
		std::bind(&GoalkeeperAnalysis::gazeEstimationFunction, this, std::placeholders::_1),
		std::bind(&GoalkeeperAnalysis::resetGazeEstimationFilters, this), CalibrationEvaluationMode::BOTH);
	evaluation.writeToFile(user_folder + "/calibrationResultsCombined.txt");
	evaluation.writeImage(user_folder + "/calibrationResultsCombined.png");


	if (use_noise_reduction_filter) {
		filter_le_coc.reset();
		filter_le_pupil.reset();
		filter_ri_coc.reset();
		filter_ri_pupil.reset();

		gaze_estimation_left->setCorneaCenterFilter(std::bind(&Vec3MedianFilter::newSample, &filter_le_coc, std::placeholders::_1));
		gaze_estimation_left->setPupilCenterFilter(std::bind(&Vec3MedianFilter::newSample, &filter_le_pupil, std::placeholders::_1));
		gaze_estimation_right->setCorneaCenterFilter(std::bind(&Vec3MedianFilter::newSample, &filter_ri_coc, std::placeholders::_1));
		gaze_estimation_right->setPupilCenterFilter(std::bind(&Vec3MedianFilter::newSample, &filter_ri_pupil, std::placeholders::_1));
	}
}

/*
void GoalkeeperAnalysis::userDirectoryChanged(const QString& text)
{
	QString trimmed = text.trimmed();
	if (trimmed.isEmpty())
	{
		// ensure that this is a valid path we can use for the filesystem library.
		trimmed = "default_output";
	}

	user_folder = trimmed.toUtf8().constData();

	ensureDirectoryExists(user_folder);

	boost::filesystem::permissions(user_folder, boost::filesystem::add_perms | boost::filesystem::all_all);


	// whenever the user directory (ie the user themselves) changes, offer the selection of stimulus video
	QString stimulus_file = QFileDialog::getOpenFileName(this, tr("Save video"), "C:");
	boost::filesystem::path stimulus_path(std::string(stimulus_file.toUtf8().constData()));


	if (boost::filesystem::exists(stimulus_path))
	{
		boost::filesystem::path stimulus_target_path(user_folder + "/stimulus.avi");
		if (boost::filesystem::exists(stimulus_target_path))
		{
			QMessageBox box;
			box.setText("Warning: The stimulus output directory copy file already exists. Delete the provided directory?\n(Otherwise this will just cancel user selectiona alltogether)\nKeep in mind that other intermediate files may also exist in the directory.");
			box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
			box.setDefaultButton(QMessageBox::Cancel);
			int res = box.exec();
			if (res == QMessageBox::Ok)
			{
				qDebug() << "Removing " << stimulus_target_path.c_str();
				boost::filesystem::remove(stimulus_target_path);
			}
		}


		copyFile(std::string(stimulus_file.toUtf8().constData()), user_folder + "/stimulus.avi");
		cv::VideoCapture loader(user_folder + "/stimulus.avi");

		// video test
		cv::VideoCapture loader(std::string(stimulus_file.toUtf8().constData()));
		if (!loader.isOpened() && _ONLINE) {
			qDebug() << "Could not open the file as video";
			cv::Mat image = cv::imread(std::string(stimulus_file.toUtf8().constData()));
			if (image.data == NULL) {
				qDebug() << "Could not open the file as image";
				// Can implement wrong choice of file here
			}
			else {
				_STILLIMAGE_STIMULUS = 1;
				stillImage = new cv::Mat(image);
				lastFrameReceived = true;
				// For the codec install x264vfw or use -1 to find a working one and print it with the debug line to replace it
				online_writer = new cv::VideoWriter(user_folder + "/output2.avi", 0x7ff71f071e1e, 575, image.size(), true);
				// qDebug() << online_writer->fourcc << " << codec chosen";
			}
		}
		else if (_ONLINE) {
			// For the codec install x264vfw or use -1 to find a working one and print it with the debug line to replace it
			online_writer = new cv::VideoWriter(user_folder + "/output2.avi", 0x7ff71f071e1e, 575, cv::Size(loader.get(CV_CAP_PROP_FRAME_WIDTH), loader.get(CV_CAP_PROP_FRAME_HEIGHT)), true);
			// qDebug() << online_writer->fourcc << " << codec chosen";
			lastFrame = cv::Mat();
		}
	}

	calibrated_for_current = false;

*/

void GoalkeeperAnalysis::userDirectoryChanged(const QString& text)
{
	QString trimmed = text.trimmed();
	if (trimmed.isEmpty())
	{
		// ensure that this is a valid path we can use for the filesystem library.
		trimmed = "default_output";
	}

	user_folder = trimmed.toUtf8().constData();

	ensureDirectoryExists(user_folder);

	boost::filesystem::permissions(user_folder, boost::filesystem::add_perms | boost::filesystem::all_all);


	// whenever the user directory (ie the user themselves) changes, offer the selection of stimulus video
	QStringList stimulus_files = QFileDialog::getOpenFileNames(this, tr("Open video"), "C:");

	if (stimulus_files.size() == 1) {
		boost::filesystem::path stimulus_path(std::string(stimulus_files.first().toUtf8().constData()));

		if (boost::filesystem::exists(stimulus_path))
		{
			boost::filesystem::path stimulus_target_path(user_folder + "/stimulus.avi");
			if (boost::filesystem::exists(stimulus_target_path))
			{
				QMessageBox box;
				box.setText("Warning: The stimulus output directory copy file already exists. Delete the provided directory?\n(Otherwise this will just cancel user selectiona alltogether)\nKeep in mind that other intermediate files may also exist in the directory.");
				box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
				box.setDefaultButton(QMessageBox::Cancel);
				int res = box.exec();
				if (res == QMessageBox::Ok)
				{
					qDebug() << "Removing " << stimulus_target_path.c_str();
					boost::filesystem::remove(stimulus_target_path);
				}
			}



			cv::Mat image = cv::imread(std::string(stimulus_files.first().toUtf8().constData()));
			if (image.data == NULL) {
				qDebug() << "Could not open the file as image";
				// video test
				cv::VideoCapture loader(std::string(stimulus_files.first().toUtf8().constData()));
				if (!loader.isOpened() && _ONLINE) {
					qDebug() << "Could not open the file as video either";
				}
				else {
					copyFile(std::string(stimulus_files.first().toUtf8().constData()), user_folder + "/stimulus.avi");
					// For the codec install x264vfw or use -1 to find a working one and print it with the debug line to replace it
					online_writer = new cv::VideoWriter(user_folder + "/output2.avi", -1, 500, cv::Size(loader.get(CV_CAP_PROP_FRAME_WIDTH), loader.get(CV_CAP_PROP_FRAME_HEIGHT)), true);
					// qDebug() << online_writer->fourcc << " << codec chosen";
					lastFrame = cv::Mat();
				}
			}
			else {
				copyFile(std::string(stimulus_files.first().toUtf8().constData()), user_folder + "/stimulus" + stimulus_path.extension().string());
				_STILLIMAGE_STIMULUS = 1;
				stillImage = new cv::Mat(image);
				lastFrameReceived = true;
				stimulusStack->emplace(stimulusStack->begin(), new cv::Mat(image));

				int width = (int)GetSystemMetrics(SM_CXSCREEN);
				int height = (int)GetSystemMetrics(SM_CYSCREEN);
				// For the codec install x264vfw or use -1 to find a working one and print it with the debug line to replace it
				//online_writer = new cv::VideoWriter(user_folder + "/output2.avi", -1, 575, cv::Size(width,height), true);
				online_writer = new cv::VideoWriter(user_folder + "/output2.avi", -1, 500, cv::Size(width, height), true);
				// qDebug() << online_writer->fourcc << " << codec chosen";
			}
		}
	}
	else {
		int index = 0;
		for (const auto& file : stimulus_files) {

			boost::filesystem::path stimulus_path(std::string(file.toUtf8().constData()));

			if (boost::filesystem::exists(stimulus_path))
			{
				boost::filesystem::path stimulus_target_path(user_folder + "/stimulus" + std::to_string(index) + stimulus_path.extension().string());
				if (boost::filesystem::exists(stimulus_target_path))
				{
					QMessageBox box;
					box.setText("Warning: The stimulus output directory copy file already exists. Delete the provided directory?\n(Otherwise this will just cancel user selectiona alltogether)\nKeep in mind that other intermediate files may also exist in the directory.");
					box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
					box.setDefaultButton(QMessageBox::Cancel);
					int res = box.exec();
					if (res == QMessageBox::Ok)
					{
						qDebug() << "Removing " << stimulus_target_path.c_str();
						boost::filesystem::remove(stimulus_target_path);
					}
				}


				copyFile(std::string(file.toUtf8().constData()), user_folder + "/stimulus" + std::to_string(index++) + stimulus_path.extension().string());

				if (_ONLINE) {
					cv::Mat image = cv::imread(std::string(file.toUtf8().constData()));
					if (image.data == NULL) {
						qDebug() << "Could not open the file as image";
						// Can implement wrong choice of file here
					}
					else {
						_STILLIMAGE_STIMULUS = 1;
						stimulusStack->emplace(stimulusStack->begin(), new cv::Mat(image));
						stillImage = new cv::Mat(image);
						lastFrameReceived = true;

						int width = (int)GetSystemMetrics(SM_CXSCREEN);
						int height = (int)GetSystemMetrics(SM_CYSCREEN);
						// For the codec install x264vfw or use -1 to find a working one and print it with the debug line to replace it
//						online_writer = new cv::VideoWriter(user_folder + "/output2.avi", -1, 575, cv::Size(width,height), true);
						online_writer = new cv::VideoWriter(user_folder + "/output2.avi", -1, 500, cv::Size(width, height), true);
						//online_writer = new cv::VideoWriter(user_folder + "/output2.mp4", 0x6C, 575, cv::Size(width, height), true);
						// qDebug() << online_writer->fourcc << " << codec chosen";
					}
				}
			}
		}
	}
	calibrated_for_current = false;
	if (_ONLINE) {
		// prepare files to write
		eyeFile = new std::ofstream(user_folder + "/eyes.txt");

		(*eyeFile) << "Start: " << stimulus_meta.start_time << std::endl;
		(*eyeFile) << "timestamp " << "pupil_right " << "pupil_left " << "glint1_right " << "glint2_right " << "glint3_right "
			<< "glint1_left " << "glint2_left " << "glint3_left " << "gaze_x " << "gaze_y " << std::endl;

	}

}

bool GoalkeeperAnalysis::event(QEvent* event)
{
	if (event->type() == StopAllOngoingEvent::my_type)
	{
		stopAllOngoing();
		return true;
	}
	return QMainWindow::event(event);
}


static void onGammaChange(int v, void* ptr)
{
	auto* that = static_cast<GoalkeeperAnalysis*>(ptr);
	that->changeGamma(v);
}

static void onGainChange(int v, void* ptr)
{
	auto* that = static_cast<GoalkeeperAnalysis*>(ptr);
	that->changeGain(v);
}

static void onHWGainChange(int v, void* ptr)
{
	auto* that = static_cast<GoalkeeperAnalysis*>(ptr);
	that->changeHWGain(v);
}


void GoalkeeperAnalysis::changeGamma(int v)
{
	camera->setGamma(v);
}

void GoalkeeperAnalysis::changeGain(int v)
{
	camera->setGain(v);
}

void GoalkeeperAnalysis::changeHWGain(int v)
{
	camera->setHWGain(v);
}