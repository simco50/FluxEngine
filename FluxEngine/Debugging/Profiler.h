#pragma once
class Profiler
{
public:
	Profiler();
	Profiler(const string& name = "", const string& description = "");
	~Profiler();

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

class AutoProfiler : public Profiler
{
public:
	AutoProfiler(const string& name, const string& description = "");
	~AutoProfiler();
};

#define AUTOPROFILE(name) AutoProfiler name ## Profiler(#name)

#define AUTOPROFILE_DESC(name, description) AutoProfiler name ## Profiler(#name, description)