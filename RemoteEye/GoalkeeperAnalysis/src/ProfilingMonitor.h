#ifndef PROFILINGMONITOR_H
#define PROFILINGMONITOR_H

// temporarily moved over from camerarelay project, todo: adapt this to this environment

#include <QObject>

#include <map>
#include "utils.h"
#include "constants.h"

#include <QElapsedTimer>

#define PROFILING_ENABLED

struct ProfilingDatapoint {
	float timeTakenMs;
	FrameTimestamp startTime;

	ProfilingDatapoint () = default;
	ProfilingDatapoint(float timeTakenMs);
};

struct ProfilingInfo {
	ProfilingDatapoint* data;
	unsigned int index;
	bool isCounter;

	ProfilingInfo(bool isCounter = false);
};

class ProfilingSection {
public:
	/// Starts immediately
	ProfilingSection();

	operator ProfilingDatapoint() const;
private:
	QElapsedTimer timer;
	FrameTimestamp startTime;
};

class ProfilingMonitor
{
public:
	const int maxObjectsRetained = profilingMonitorMaxObjectsRetained;

	explicit ProfilingMonitor();
	~ProfilingMonitor();

	QString generateReport();

	void addTiming(const std::string& account, FrameTimestamp startTime, float timing);
	void addTiming(const std::string& account, ProfilingDatapoint info, bool isCounter = false);

private:
	std::map<std::string, ProfilingInfo> timings;
	QMutex timingsMutex;
	FrameTimestamp lastReportTime;
};


extern ProfilingMonitor gProfilingMonitor;

#endif // PROFILINGMONITOR_H
