#pragma once
#include "Async\Mutex.h"

enum class LogType
{
	VeryVerbose,
	Verbose,
	Info,
	Warning,
	Error,
	FatalError,
};

class File;

class Console
{
public:
	Console();
	~Console();

	struct LogEntry
	{
		LogEntry(const std::string& message, const LogType type)
			: Message(message), Type(type)
		{}
		std::string Message;
		LogType Type;
	};

	void FlushThreadedMessages();

	static bool LogHRESULT(const std::string &source, HRESULT hr);
	static bool LogHRESULT(char* source, HRESULT hr);
	static void Log(const std::string &message, LogType type = LogType::Info);
	static void LogFormat(LogType type, const char* format, ...);
	static void LogFormat(LogType type, const std::string& format, ...);
	static void SetVerbosity(LogType type);
	static bool CleanupLogs(const TimeSpan& age);
	static bool Flush();

	static const std::deque<LogEntry>& GetHistory();

private:
	void InitializeConsoleWindow();

	char* m_ConvertBuffer;
	const size_t m_ConvertBufferSize = 4096;
	std::queue<LogEntry> m_MessageQueue;
	Mutex m_QueueMutex;

	LogType m_Verbosity = LogType::Info;
	File* m_pFileLog;
	HANDLE m_ConsoleHandle = nullptr;

	std::deque<LogEntry> m_History;
};
