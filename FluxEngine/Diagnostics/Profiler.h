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

class AutoProfilerBlock : public Profiler
{
public:
	AutoProfilerBlock(const string& name, const string& description = "");
	~AutoProfilerBlock();

private:
};

#define PROFILING
#ifdef PROFILING

#define AUTOPROFILE(name) AutoProfilerBlock name ## AutoProfilerBlock(#name)
#define AUTOPROFILE_DESC(name, description) AutoProfilerBlock name ## AutoProfilerBlock(#name, description)

#else

#define AUTOPROFILE(name) 0
#define AUTOPROFILE_DESC(name, description) 0

#endif