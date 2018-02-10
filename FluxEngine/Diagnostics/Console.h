#pragma once

enum class LogType
{
	INFO,
	WARNING,
	ERROR,
};

class IFile;

class Console
{
public:
	Console();
	~Console();

	static void Initialize();
	static void Release();

	static bool LogFmodResult(FMOD_RESULT result);
	static bool LogHRESULT(const std::string &source, HRESULT hr);
	static bool LogHRESULT(char* source, HRESULT hr);
	static void Log(const std::string &message, LogType type = LogType::INFO);
	static void LogWarning(const std::string &message);
	static void LogError(const std::string &message);
	static void LogFormat(LogType type, const char* format, ...);
	static void LogFormat(LogType type, const std::string& format, ...);

private:
	static void InitializeConsoleWindow();
	static std::string GetTime();

	static char* m_ConvertBuffer;
	static const size_t m_ConvertBufferSize = 2048;

	static IFile* m_pFileLog;
	static HANDLE m_ConsoleHandle;
};
