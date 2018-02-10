#pragma once

class PerfTimer
{
public:
	PerfTimer();
	PerfTimer(const std::string& name = "", const std::string& description = "");
	~PerfTimer();

	void Start();
	float Stop();

protected:
	std::string m_Name;
	std::string m_Description;

private:
	double m_SecondsPerCount = 0.0f;

	__int64 m_StartTime = 0;
	__int64 m_StopTime = 0;
};

class AutoPerfTimer : public PerfTimer
{
public:
	AutoPerfTimer(const std::string& name, const std::string& description = "");
	~AutoPerfTimer();
};