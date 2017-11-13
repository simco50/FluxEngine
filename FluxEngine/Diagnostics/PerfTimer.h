#pragma once

class PerfTimer
{
public:
	PerfTimer();
	PerfTimer(const string& name = "", const string& description = "");
	~PerfTimer();

	void Start();
	float Stop();

protected:
	string m_Name;
	string m_Description;

private:
	double m_SecondsPerCount = 0.0f;

	__int64 m_StartTime = 0;
	__int64 m_StopTime = 0;
};

class AutoPerfTimer : public PerfTimer
{
public:
	AutoPerfTimer(const string& name, const string& description = "");
	~AutoPerfTimer();

private:
};