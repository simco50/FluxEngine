#include "stdafx.h"
#include <time.h>
#include <iomanip>

using namespace std;

wofstream* DebugLog::m_pFileLog = nullptr;
HANDLE DebugLog::m_ConsoleHandle;
wchar_t* DebugLog::m_ConvertBuffer = new wchar_t[m_ConvertBufferSize];

DebugLog::DebugLog()
{}

DebugLog::~DebugLog()
{
}

void DebugLog::Initialize()
{
	m_pFileLog = new wofstream();
	m_pFileLog->open("DebugLog.log", ios::app);
	*m_pFileLog << endl << "-------------FLUX ENGINE LOG START--------------" << endl << endl;

	time_t timer;
	time(&timer);
	tm localTime;
	localtime_s(&localTime, &timer);
	wstringstream stream;
	*m_pFileLog << L"Date: " << localTime.tm_mday << L"-" << localTime.tm_mon + 1 << L"-" << 1900 + localTime.tm_year << endl;
	*m_pFileLog << L"Time: " << GetTime() << endl;

	m_ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}

void DebugLog::Release()
{
	*m_pFileLog  << endl << "--------------FLUX ENGINE LOG END---------------" << endl;
	delete m_pFileLog;
	delete[] m_ConvertBuffer;
}

bool DebugLog::LogFmodResult(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		wstringstream stream;
		stream << L"FMOD Error (" << result << ") " << FMOD_ErrorString(result);
		Log(stream.str(), LogType::ERROR);
		return true;
	}
	return false;
}

bool DebugLog::LogHRESULT(const std::wstring &source, HRESULT hr)
{
	if (FAILED(hr))
	{
		if (FACILITY_WINDOWS == HRESULT_FACILITY(hr))
			hr = HRESULT_CODE(hr);

		std::wstringstream ss;
		if (source.size() != 0)
		{
			ss << "Source: ";
			ss << source;
			ss << "\n";
		}
		ss << "Message: ";

		TCHAR* errorMsg;
		if (FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&errorMsg, 0, nullptr) != 0)
		{
			ss << errorMsg;
		}
		else
		{
			ss << "Could not find a description for error: '";
			ss << hr;
			ss << "'.";
		}

		Log(ss.str(), LogType::ERROR);
		return true;
	}

	return false;
}

bool DebugLog::LogHRESULT(char* source, HRESULT hr)
{
	string msg(source);
	wstring wMsg(msg.begin(), msg.end());
	return LogHRESULT(wMsg, hr);
}

void DebugLog::Log(const std::wstring &message, LogType type)
{
	wstringstream stream;
	stream << L"[" << GetTime() << L"]";
	switch (type)
	{
	case LogType::INFO:
		stream << L"[INFO] ";
		break;
	case LogType::WARNING:
		SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		stream << L"[WARNING] ";
		break;
	case LogType::ERROR:
		SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
		stream << L"[ERROR] ";
		break;
	default:
		break;
	}

	stream << message;
	wcout << stream.str() << endl;

	(*m_pFileLog) << stream.str() << endl;
	m_pFileLog->flush();
	SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

	if (type == LogType::ERROR)
	{
		MessageBoxW(nullptr, message.c_str(), L"Fatal Error", MB_ICONINFORMATION);
		PostQuitMessage(-1);
	}
}

void DebugLog::LogFormat(LogType type, const wchar_t* format, ...)
{
	va_list ap;

	va_start(ap, format);
	_vsnwprintf_s(&m_ConvertBuffer[0], m_ConvertBufferSize, m_ConvertBufferSize, format, ap);
	va_end(ap);
	Log(wstring(&m_ConvertBuffer[0]), type);
}

wstring DebugLog::GetTime()
{
	time_t timer;
	time(&timer);
	tm localTime;
	localtime_s(&localTime, &timer);
	wstringstream stream;
	stream << setfill(L'0');
	stream << setw(2) << localTime.tm_hour << ":" << setw(2) << localTime.tm_min << ":" << setw(2) << localTime.tm_sec;
	return stream.str();
}
