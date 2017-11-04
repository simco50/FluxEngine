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

	unsigned int GetDepth() const { return m_Depth; }

private:
	unsigned int m_Depth = 0;
};

#ifdef PROFILING

#define AUTOPROFILE(name) AutoProfiler name ## Profiler(#name)
#define AUTOPROFILE_DESC(name, description) AutoProfiler name ## Profiler(#name, description)

#else

#define AUTOPROFILE(name) 0
#define AUTOPROFILE_DESC(name, description) 0

#endif