#pragma once
class PerfTimer
{
public:
	PerfTimer(const string& source = "");
	~PerfTimer();

	float Stop();

private:
	double m_SecondsPerCount = 0.0f;

	__int64 m_StartTime = 0;
	__int64 m_StopTime = 0;

	string m_Source;
};

