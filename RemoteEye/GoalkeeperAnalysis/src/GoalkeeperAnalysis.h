#ifndef GOALKEEPER_ANALYSIS_H_
#define GOALKEEPER_ANALYSIS_H_

#include <memory>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QMainWindow>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


#define _ONLINE 1

#include "constants.h"
#include "IntermediateTypes.h"
#include "../../../GazeEstimationCpp/GazeEstimationTypes.hpp"
#include "../../../GazeEstimationCpp/OneCameraSpherical.hpp"
#include "PupilDetection.h"
#include "SparseInputImage.h"
#include "CalibrationPointAggregates.h"
#include <MAFilter.h>
#include "kalman.h"
#include <fstream>
namespace Ui
{
	class GoalkeeperAnalysisClass;
}

class VideoCaptureSession;
class VideoWriterThread;
class CalibrationDialog;
class VideoFilePollingThread;
class uEyeCamera;
class LiveGazeDisplay;
class CalibrationTrainer;
class InputFeaturesDisplay;
class Vec3MedianFilter;

enum class OperationMode
{
	NONE = 0,
	SAVING,
	SAVING_FULL_IMAGE,
	CALIBRATION,
	CALIBRATION_ACCURACY,
	EYEIMAGES,
	LIVE_TESTING,
	LIVE_GAZE
};

class GoalkeeperAnalysis : public QMainWindow
{
	Q_OBJECT
public:
	explicit GoalkeeperAnalysis(QWidget* parent = Q_NULLPTR);
	~GoalkeeperAnalysis();
	
	void receiveFrame(const cv::Mat& frame);
	PupilGlintCombiInstance lowPassFilter(DetectedFeatures features);
	PupilGlintCombiInstance filterFeatures(PupilGlintCombiInstance detectedFeatures);


	MAFilter gaze_maFilter;
	MAFilter pupil_right_maFilter;
	MAFilter pupil_left_maFilter;

	ScreenspaceGazeResult lowPassGaze(ScreenspaceGazeResult gaze);
	void showStimulusFrame(const cv::Mat& frame, unsigned int frame_index, double time);
	void stimulusPlaybackEnded();
		
	bool event(QEvent* event) override;

	void changeGamma(int v);
	void changeGain(int v);
	void changeHWGain(int v);

	 cv::Point2f currentGlint1;
	 cv::Point2f  currentGlint2;
	 cv::Point2f  currentGlint3;

	cv::Point2f  currentPupilRight;
	cv::Point2f  currentPupilLeft;



	cv::Point2f old_gaze;

public slots:
	void userDirectoryChanged(const QString& text);
	void calibrationFinished();
	void calibrationAccuracyFinished();
	void liveGazeDisplayClosed();

private slots:
	/// The corresponding action to the start capture button, not to be confused with start capture.
	void startCaptureSession();
	void startCalibration();
	void startSavingEyevideo();
	void startEvaluatingCalibrationAccuracy();
	void startPlaybackSession();
	void startLiveTesting();
	void startCaptureFullImage();

	void stopAllOngoing();

	void refreshProfilingInfo();
	void resetGazeEstimationFilters();

private:
	void calibrateFrom(const CalibrationData& data);
	void setupDefaultSetup();
	void setupCapture(OperationMode target_mode);
	void writeCalibrationRawDataToFile(CalibrationData data);


	CalibrationTrainer* createTrainer(PupilDetection* wrapper);
	CalibrationTrainer* createTrainer(BoreWrapper* wrapper);
	
	PupilGlintCombiInstance detectRaw(const cv::Mat& frame);
	PupilGlintCombiInstance detectRaw(const SparseInputImage& frame);

	ScreenspaceGazeResult gazeEstimationFunction(const DetectedFeatures& instance);
	ScreenspaceGazeResult singleLeftGazeEstimationFunction(const DetectedFeatures& instance);
	ScreenspaceGazeResult singleRightGazeEstimationFunction(const DetectedFeatures& instance);
		
	OperationMode current_mode;

	std::unique_ptr<Ui::GoalkeeperAnalysisClass> ui;
	std::unique_ptr<VideoCaptureSession> capture;
	std::unique_ptr<VideoWriterThread> video_recorder;
	std::unique_ptr<CalibrationDialog> calibration_dialog;
	std::unique_ptr<CalibrationTrainer> calibration_trainer;
	std::unique_ptr<VideoCaptureSession> stimulus_playback;
	std::unique_ptr<uEyeCamera> camera;
	std::unique_ptr<LiveGazeDisplay> live_display;
	std::unique_ptr<PupilDetection> pupil_detection;
	std::unique_ptr<InputFeaturesDisplay> input_features_display;


	VideoFrame2Timestamp stimulus_meta;
	/// The true position and size that the stimulus video were played at.
	cv::Rect stimulus_position;

	cv::Size video_frame_size;
	double video_framerate;

	int first_eye_threshold;

	std::string user_folder;
	std::string stimulus_file;

	unsigned int frame_counter;
	bool display_found_features = false;									// Deactivated for speed up

	std::atomic<bool> new_frame_processing_lockout = false;

	/// has there been a successful calibration since the user was last changed
	bool calibrated_for_current;
	std::unique_ptr<gazeestimation::OneCamSphericalGE> gaze_estimation_right;
	std::unique_ptr<gazeestimation::OneCamSphericalGE> gaze_estimation_left;
	gazeestimation::EyeAndCameraParameters setup_parameters_right;
	gazeestimation::EyeAndCameraParameters setup_parameters_left;
	gazeestimation::Vec2 screen_resolution;
	gazeestimation::Vec2 screen_pixel_size_cm;
	double z_shift;
	gazeestimation::Vec3 wcs_offset;

	int gamma_slider_value = intial_gamma;
	int gain_slider_value = initial_gain;
	int hw_gain_slider_value = initial_hw_gain;

	QTimer* perfStatsRefreshTimer;

	Vec3MedianFilter filter_ri_pupil, filter_ri_coc;
	Vec3MedianFilter filter_le_pupil, filter_le_coc;


	const cv::String stimulus_window_id = "Stimulus Plaback";
	const cv::String control_window_id = "Main Control";



	cv::VideoWriter* online_writer;
	cv::Point2f* lastGaze;
	cv::Mat lastFrame;
	bool lastFrameReceived;
	bool _STILLIMAGE_STIMULUS;
	
	std::vector<cv::Mat*> *stimulusStack;
	cv::Mat *stillImage;
	QAction *stimulusEndHotkey;

	int frameCount;
	MAFilter pupilR_maFilter;
	MAFilter pupilL_maFilter;
	PupilGlintCombiInstance pgci;
	std::ofstream *eyeFile; 
	ScreenspaceGazeResult getGazeAndWriteToFile(cv::Mat,
		std::ofstream*,
		DetectionToGazeEstimation,
		PupilDetection*,
		int);
};

#endif