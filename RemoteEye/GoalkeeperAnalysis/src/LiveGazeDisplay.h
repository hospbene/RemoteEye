#ifndef LIVE_GAZE_DISPLAY_H_
#define LIVE_GAZE_DISPLAY_H_

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QWidget>
#include "GazeEstimationAdapters.hpp"
#ifdef _MSC_VER
#pragma warning(pop)
#endif



class LiveGazeDisplay : public QWidget
{
	Q_OBJECT
public:
	explicit LiveGazeDisplay(double target_fps, QWidget* parent = nullptr);
	~LiveGazeDisplay();

protected:
	void paintEvent(QPaintEvent* event) override;
	bool event(QEvent* event) override;

public slots:
	void setGaze(QPoint gaze_left, QPoint gaze_right, QPoint gaze_combined);

signals:
	/// Signals that the user is no longer interested in the output of this. Can be emitted multiple times
	/// over the lifetime of this object.
	void ended();

private:
	bool isOffscreen(QPoint point) const;
	static void drawGaze(QPainter* painter, QPoint point, QColor color);

	QRect screen_parameters;

	QPoint last_gaze_left;
	QPoint last_gaze_right;
	QPoint last_gaze_total;

	FrameTimestamp last_render;
	double desired_frame_interval;
};


#endif
