#include "LiveGazeDisplay.h"


#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QDesktopWidget>
#include <QApplication>
#include <QPainter>
#include "utils.h"
#include "ProfilingMonitor.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

class DisplayNewGazeEvent : public QEvent
{
public:
	static const QEvent::Type my_type = static_cast<QEvent::Type>(QEvent::User + 85);
	explicit DisplayNewGazeEvent() : QEvent(my_type) {}
};

LiveGazeDisplay::LiveGazeDisplay(double target_fps, QWidget* parent):
	QWidget(parent),
	last_render(getCurrentTime())
{
	desired_frame_interval = (1000 / target_fps) - 1; // make some allowances for the time it takes to draw
	
	setWindowFlags(Qt::FramelessWindowHint);
	setWindowOpacity(0.5);
	setStyleSheet("QWidget{ background: #444444 }");
	screen_parameters = QApplication::desktop()->screenGeometry();;
	resize(screen_parameters.width(), screen_parameters.height());
	move(0, 0);
}

LiveGazeDisplay::~LiveGazeDisplay()
{
	emit ended();
}

void LiveGazeDisplay::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	if (!isOffscreen(last_gaze_left))
		drawGaze(&painter, last_gaze_left, QColor(255, 0, 0));
	if (!isOffscreen(last_gaze_right))
		drawGaze(&painter, last_gaze_right, QColor(0, 0, 255));
	if (!isOffscreen(last_gaze_total))
		drawGaze(&painter, last_gaze_total, QColor(0, 255, 0));

	last_render = getCurrentTime();
	gProfilingMonitor.addTiming("LiveGazeDisplay_paint", ProfilingSection(), true);
}

bool LiveGazeDisplay::event(QEvent* event)
{
	if (event->type() == QEvent::KeyPress || event->type() == QEvent::Close){
		this->close();
		emit ended();
		return true;
	}
	if(event->type() == DisplayNewGazeEvent::my_type)
	{
		this->repaint();
	}
	return QWidget::event(event);
}

void LiveGazeDisplay::setGaze(QPoint gaze_left, QPoint gaze_right, QPoint gaze_combined)
{
	last_gaze_left = gaze_left;
	last_gaze_right = gaze_right;
	last_gaze_total = gaze_combined;

	// Don't unneccessarily spam the event loop with events
	if(getCurrentTime() - last_render > desired_frame_interval)
	{
		QApplication::postEvent(this, new DisplayNewGazeEvent(), Qt::HighEventPriority);
	}
}

bool LiveGazeDisplay::isOffscreen(QPoint point) const
{
	return point.x() <= 0 || point.y() <= 0 ||
		point.x() >= screen_parameters.width() ||
		point.y() > screen_parameters.height();
}

void LiveGazeDisplay::drawGaze(QPainter* painter, QPoint point, QColor color)
{
	const int halfsize = 5;
	painter->setBrush(color);
	painter->drawEllipse(point.x()-halfsize, point.y()-halfsize, 2 * halfsize, 2 * halfsize);
}
