#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QDialog>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


#include <boost/thread/thread_only.hpp>

#include "GoalkeeperAnalysis.h"
#include <mutex>

#include "IntermediateTypes.h"


namespace Ui {
class CalibrationDialog;
}

class CalibrationTrainer;
class CalibrationSequenceRunner;

/// \brief Calibration Dialog that displays several points in order
/// 
/// This class manages the entire calibration sequence, the only data inputs are
/// * receiveEyeFeatures - if do_training has been set to false
/// * receiveRawImage - if do_training has been set to true
/// this information is available via needsRawImages
class CalibrationDialog : public QDialog
{
    Q_OBJECT

public:
	/// @param do_training @see TrainingDataStore
    explicit CalibrationDialog(const std::vector<std::pair<double, double>>& calibration_points, 
		std::pair<unsigned int, unsigned int> size, 
		CalibrationTrainer* trainer, 
		QWidget *parent = 0);
    ~CalibrationDialog();

	bool event(QEvent* event) override;
	CalibrationData getCalibratedData();
	void receiveEyeFeatures(PupilGlintCombiInstance features);
	void receiveRawImage(QueuedFrame& frame);

	bool needsDetectedFeatures() const;
signals:
	void calibrationFinished();

private:
	std::mutex data_buffer_mutex;
    Ui::CalibrationDialog *ui;
	std::unique_ptr<CalibrationSequenceRunner> runner;
	boost::thread thread_sequence;

	CalibrationData collected_data;
	// Contains the received data. Not all of it is actually be used, see the event function for details.
	std::vector<PupilGlintCombiInstance> data_buffer;
	std::pair<float, float> current_point;
	FrameTimestamp start_of_current_point;

	CalibrationTrainer* training;
	bool has_been_consumed;

};

#endif // CALIBRATIONDIALOG_H
