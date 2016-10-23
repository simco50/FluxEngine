#pragma once

enum class LogType
{
	INFO,
	WARNING,
	ERROR,
};

class DebugLog
{
public:
	DebugLog();
	~DebugLog();

	static void Initialize();
	static void Release();

	static bool LogFmodResult(FMOD_RESULT result);
	static bool LogHRESULT(const std::wstring &source, HRESULT hr);
	static bool LogHRESULT(char* source, HRESULT hr);
	static void Log(const std::wstring &message, LogType type = LogType::INFO);
	static void LogFormat(LogType type, const wchar_t* format,...);

private:
	static wstring GetTime();

	static wchar_t* m_ConvertBuffer;
	static const size_t m_ConvertBufferSize = 1024;

	static wofstream* m_pFileLog;
	static HANDLE m_ConsoleHandle;
};