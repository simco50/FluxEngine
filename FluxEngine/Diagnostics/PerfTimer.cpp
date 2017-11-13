#include "stdafx.h"
#include "PerfTimer.h"

PerfTimer::PerfTimer() :
	m_Name("")
{

}

PerfTimer::PerfTimer(const string& name, const string& description) :
	m_Name(name),
	m_Description(description)
{

}

PerfTimer::~PerfTimer()
{
}

void PerfTimer::Start()
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_SecondsPerCount = 1000.0f / (double)(countsPerSec);

	QueryPerformanceCounter((LARGE_INTEGER*)&m_StartTime);
}

float PerfTimer::Stop()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&m_StopTime);
	return (float)((m_StopTime - m_StartTime) * m_SecondsPerCount);
}

AutoPerfTimer::AutoPerfTimer(const string& name, const string& description) :
	PerfTimer(name, description)
{
	Start();
}

AutoPerfTimer::~AutoPerfTimer()
{
	const float time = Stop();
	stringstream str;
	str << "[" << m_Name << "] " << m_Description << " > " << time << " ms";
	FLUX_LOG(INFO, str.str());
}
