#pragma once
#include "Helpers/Singleton.h"
#include "External/NlohmannJson/json.hpp"
#include "Async/Thread.h"

class ProfilingThread : public Thread
{
public:
	ProfilingThread();

	void Run();
	void Shutdown();

	void BeginEvent(const std::string& name, const std::string& description);
	void EndEvent(const std::string& name, const std::string& description);
	void MarkDuration(const std::string& name, const std::string& description, const int64 startTime);
	void MarkEvent(const std::string& name, const std::string& description);

	void DumpStatsToFile(const std::string& filePath);
	bool IsCapturing() const;

private:
	enum class EventType
	{
		Begin,
		End,
		Event,
		Duration,
		MAX
	};

	struct QueuedEventData
	{
		std::string Name;
		std::string Description;
		uint32 Thread;
		int64 Time = 0;
		int64 Duration = 0;
		int Frame;
		EventType Type = EventType::MAX;
	};

	virtual int ThreadFunction() override;
	void WriteToFile();
	void AddEvent(const std::string& name, const std::string& description, const EventType type, const int64 startTime = 0);

	int64 m_Frequency;
	bool m_Shutdown = false;
	bool m_Complete = false;
	bool m_Dumping = false;
	int m_JobsToCompleteUntilDump = -1;

	Mutex m_JobMutex;

	nlohmann::json m_JsonString;
	std::string m_LogFilePath;
	std::queue<QueuedEventData> m_Jobs;
};

class Profiler : public Singleton<Profiler>
{
public:
	Profiler();
	~Profiler();

	void Tick();
	void Capture(int frameCount = 1);

	void BeginEvent(const std::string& name, const std::string& description = "");
	void EndEvent(const std::string& name, const std::string& description = "");
	void MarkDuration(const std::string& name, const int64 startTime, const std::string& description = "");
	void MarkEvent(const std::string& name, const std::string& description = "");

private:
	bool ShouldRecord() const;
	int m_FrameToCapture = -1;
	int m_FramesToCapture = 0;

	std::unique_ptr<ProfilingThread> m_Thread;
};

class AutoProfiler
{
public:
	AutoProfiler(const std::string& name, const std::string& description = "") :
		AutoProfiler(name.c_str(), description.c_str())
	{}

	AutoProfiler(const char* name, const char* description = "")
	{
		memcpy(m_Name, name, strlen(name) + 1);
		memcpy(m_Description, description, strlen(description) + 1);
		Profiler::Instance()->BeginEvent(m_Name, m_Description);
	}
	~AutoProfiler()
	{
		Profiler::Instance()->EndEvent(m_Name, m_Description);
	}

private:
	char m_Name[64];
	char m_Description[128];
};

#ifdef PROFILING
#define AUTOPROFILE(name) AutoProfiler Profiler_##name(#name)
#define AUTOPROFILE_DESC(name, desc) AutoProfiler Profiler_##name(#name, desc)
#define PROFILER_EVENT(name) Profiler::Instance()->MarkEvent(#name)
#define PROFILER_EVENT_DESC(name, desc) Profiler::Instance()->MarkEvent(#name, desc)
#else
#define AUTOPROFILE(name)
#define AUTOPROFILE_DESC(name, desc)
#define PROFILER_EVENT(name)
#define PROFILER_EVENT_DESC(name, desc)
#endif