#include "ProfilingMonitor.h"
#include "utils.h"

// temporarily moved over from camerarelay project, todo: adapt this to this environment

ProfilingMonitor gProfilingMonitor;

ProfilingMonitor::ProfilingMonitor()
{
}

ProfilingMonitor::~ProfilingMonitor() {
	for (auto kv = timings.begin(); kv != timings.end(); ++kv) {
		delete[] kv->second.data;
	}
}

QString ProfilingMonitor::generateReport() {
#ifdef PROFILING_ENABLED
	FrameTimestamp thisTime = getCurrentTime();
	//TODO: proper copy
	QMutexLocker ml(&timingsMutex);
	QString result;
	for (auto it = timings.begin(); it != timings.end(); ++it) {
		ProfilingInfo info = it->second;

		if (it->second.isCounter) {
			double eps = (1000. * it->second.data[0].timeTakenMs) / (thisTime - lastReportTime);
			it->second.data[0].timeTakenMs = 0;
			result.append(it->first.c_str()).append(": ").append(QString::number(eps)).append(" eps\n");
		}
		else {
			float totalTime = 0.f;
			for (int i = 0; i < maxObjectsRetained; i++) {
				totalTime += info.data[(info.index + i) % maxObjectsRetained].timeTakenMs;
			}
			float avgTime = totalTime / maxObjectsRetained;

			double variance = 0;
			for (int i = 0; i < maxObjectsRetained; i++) {
				variance += pow(info.data[(info.index + i) % maxObjectsRetained].timeTakenMs - avgTime, 2);
			}
			variance /= maxObjectsRetained;
			double std = sqrt(variance);

			result.append(it->first.c_str()).append(": ").append(QString::number(avgTime)).append("ms +- ").append(QString::number(std)).append("\n");
		}

	}
	lastReportTime = getCurrentTime();
	return result;
#endif
	return "Profiling disabled.";
}

void ProfilingMonitor::addTiming(const std::string &account, FrameTimestamp startTime, float timing) {
#ifdef PROFILING_ENABLED
	ProfilingDatapoint datapoint;
	datapoint.startTime = startTime;
	datapoint.timeTakenMs = timing;
	addTiming(account, datapoint);
#endif
}
void ProfilingMonitor::addTiming(const std::string &account, ProfilingDatapoint datapoint, bool isCounter) {
#ifdef PROFILING_ENABLED
	QMutexLocker ml(&timingsMutex);

	auto foundAccount = timings.find(account);
	if (foundAccount == timings.end()) {
		ProfilingInfo info;
		info.data = new ProfilingDatapoint[maxObjectsRetained];
		info.data[0].timeTakenMs = 0;
		info.isCounter = isCounter;
		foundAccount = timings.insert(std::make_pair(account, info)).first;
	}
	if (foundAccount->second.isCounter) {
		foundAccount->second.data[foundAccount->second.index].timeTakenMs = foundAccount->second.data[foundAccount->second.index].timeTakenMs + 1;
		foundAccount->second.index = 0;
	}
	else {
		foundAccount->second.data[foundAccount->second.index] = datapoint;
		foundAccount->second.index = (foundAccount->second.index + 1) % maxObjectsRetained;
	}
#endif
}


ProfilingDatapoint::ProfilingDatapoint(float timeTakenMs):
startTime(getCurrentTime()),
timeTakenMs(timeTakenMs)
{
	
}

ProfilingInfo::ProfilingInfo(bool isCounter) :
	data(nullptr),
	index(0),
	isCounter(isCounter) {

}

ProfilingSection::operator ProfilingDatapoint() const {
	ProfilingDatapoint data;
	data.timeTakenMs = (float)timer.nsecsElapsed() / 1e6;
	data.startTime = startTime;
	return data;
}

ProfilingSection::ProfilingSection() {
	startTime = getCurrentTime();
	timer.start();
}
