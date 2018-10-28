#include "FluxEngine.h"
#include "Profiler.h"
#include "FileSystem\File\PhysicalFile.h"
#include "Async\Thread.h"
#include "Core\CommandLine.h"

ProfilingThread::ProfilingThread()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_Frequency);
}

void ProfilingThread::Shutdown()
{
	m_Shutdown = true;

	while (m_Complete == false)
	{
		Sleep(0);
	}
}

void ProfilingThread::BeginEvent(const std::string& name, const std::string& description)
{
	AddEvent(name, description, EventType::Begin, 0);
}

void ProfilingThread::EndEvent(const std::string& name, const std::string& description)
{
	AddEvent(name, description, EventType::End, 0);
}

void ProfilingThread::MarkDuration(const std::string& name, const std::string& description, const int64 startTime)
{
	AddEvent(name, description, EventType::Duration, startTime);
}

void ProfilingThread::MarkEvent(const std::string& name, const std::string& description)
{
	AddEvent(name, description, EventType::Event, 0);
}

void ProfilingThread::DumpStatsToFile(const std::string& filePath)
{
	if (m_Dumping == false)
	{
		m_LogFilePath = filePath;
		m_JobsToCompleteUntilDump = (int)m_Jobs.size();
		m_Dumping = true;
	}
}

bool ProfilingThread::IsCapturing() const
{
	return m_Dumping;
}

int ProfilingThread::ThreadFunction()
{
	while (m_Shutdown == false)
	{
		if (m_Dumping)
		{
			if (m_JobsToCompleteUntilDump <= 0)
			{
				WriteToFile();
				m_Dumping = false;
			}
			--m_JobsToCompleteUntilDump;
		}

		if (m_Jobs.size() == 0)
		{
			Sleep(0);
			continue;
		}

		m_JobMutex.Lock();
		QueuedEventData j = m_Jobs.front();
		m_Jobs.pop();
		m_JobMutex.Unlock();

		std::map<std::string, nlohmann::json> data;
		data["name"] = j.Name;
		data["cat"] = j.Name.substr(0, j.Name.find('_'));
		data["pid"] = "FluxEngine";
		data["tid"] = IsMainThread(j.Thread) ? "Main" : "Worker " + std::to_string(j.Thread);
		data["ts"] = j.Time * 1000000 / m_Frequency;

		std::map<std::string, std::string> args;

		switch (j.Type)
		{
		case EventType::Begin:
			data["ph"] = "B";
			break;
		case EventType::End:
			data["ph"] = "E";
			break;
		case EventType::Event:
			data["ph"] = "i";
			break;
		case EventType::Duration:
			data["ph"] = "X";
			data["dur"] = std::to_string(j.Duration * 1000000 / m_Frequency);
			args["duration"] = std::to_string(j.Duration * 1000 / m_Frequency) + " ms";
		default:
			break;
		}

		args["description"] = j.Description;
		args["frame"] = std::to_string(j.Frame);

		data["args"] = args;

		m_JsonString["traceEvents"].push_back(data);
	}

	m_Complete = true;
	return 0;
}

void ProfilingThread::WriteToFile()
{
	std::unique_ptr<PhysicalFile> pFile = std::make_unique<PhysicalFile>(m_LogFilePath);
	if (pFile->OpenWrite(false, false))
	{
		m_JsonString["displayTimeUnit"] = "ms";
		m_JsonString["systemTraceEvents"] = "SystemTraceData";

		std::string o = m_JsonString.dump();
		pFile->WriteLine(o);
		pFile->Close();

		m_JsonString.clear();
	}
}


void ProfilingThread::AddEvent(const std::string& name, const std::string& description, const EventType type, const int64 startTime /*= 0*/)
{
	QueuedEventData data;
	data.Name = name;
	data.Description = description;
	data.Thread = Thread::GetCurrentId();
	data.Frame = GameTimer::Ticks();
	data.Type = type;

	switch (type)
	{
	case EventType::Begin:
	case EventType::End:
	case EventType::Event:
		QueryPerformanceCounter((LARGE_INTEGER*)&data.Time);
		break;
	case EventType::Duration:
		int64 now;
		QueryPerformanceCounter((LARGE_INTEGER*)&now);
		data.Duration = now - startTime;
		data.Time = startTime;
		break;
	case EventType::MAX:
	default:
		throw;
	}
	ScopeLock lock(m_JobMutex);
	m_Jobs.push(data);
}

Profiler::Profiler()
{
	if (CommandLine::GetBool("CaptureFirstFrame"))
	{
		m_FrameToCapture = 0;
		m_FramesToCapture = 1;
	}

	m_Thread = std::make_unique<ProfilingThread>();
	m_Thread->RunThread();
}

Profiler::~Profiler()
{
	m_Thread->Shutdown();
}

void Profiler::Tick()
{
	if (m_FrameToCapture >= 0 && m_FrameToCapture + m_FramesToCapture < GameTimer::Ticks() + 1)
	{
		TimeStamp time;
		DateTime::Now().Split(time);

		std::string filePath = Printf("%s\\Profiling_%02d-%02d-%02d-%02d-%02d-%02d_%d.json",
			Paths::ProfilingDir().c_str(),
			time.Year,
			time.Month,
			time.Day,
			time.Hour,
			time.Minute,
			time.Second,
			0);

		m_Thread->DumpStatsToFile(filePath);
		m_FrameToCapture = -1;
	}
}

void Profiler::Capture(int frameCount /*= 1*/)
{
	if (m_Thread->IsCapturing() == false)
	{
		m_FrameToCapture = GameTimer::Ticks() + 1;
		m_FramesToCapture = frameCount;
	}
}

void Profiler::BeginEvent(const std::string& name, const std::string& description) const
{
	if (ShouldRecord())
	{
		m_Thread->BeginEvent(name, description);
	}
}

void Profiler::EndEvent(const std::string& name, const std::string& description) const
{
	if (ShouldRecord())
	{
		m_Thread->EndEvent(name, description);
	}
}

void Profiler::MarkEvent(const std::string& name, const std::string& description /*= ""*/) const
{
	if (ShouldRecord())
	{
		m_Thread->MarkEvent(name, description);
	}
}

void Profiler::MarkDuration(const std::string& name, const int64 startTime, const std::string& description /*= ""*/) const
{
	if (ShouldRecord())
	{
		m_Thread->MarkDuration(name, description, startTime);
	}
}

bool Profiler::ShouldRecord() const
{
	int frame = GameTimer::Ticks();
	if (frame < m_FrameToCapture || frame >= m_FrameToCapture + m_FramesToCapture)
	{
		return false;
	}
	return true;
}