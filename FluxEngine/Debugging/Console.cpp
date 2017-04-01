#include "stdafx.h"
#include <time.h>
#include <iomanip>

using namespace std;

ofstream* Console::m_pFileLog = nullptr;
HANDLE Console::m_ConsoleHandle;
char* Console::m_ConvertBuffer = new char[m_ConvertBufferSize];

Console::Console()
{}

Console::~Console()
{
}

void Console::Initialize()
{
	m_pFileLog = new ofstream();
	m_pFileLog->open("DebugLog.log", ios::app);
	*m_pFileLog << endl << "-------------FLUX ENGINE LOG START--------------" << endl << endl;

	time_t timer;
	time(&timer);
	tm localTime;
	localtime_s(&localTime, &timer);
	wstringstream stream;
	*m_pFileLog << "Date: " << localTime.tm_mday << "-" << localTime.tm_mon + 1 << "-" << 1900 + localTime.tm_year << endl;
	*m_pFileLog << "Time: " << GetTime() << endl;

	m_ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}

void Console::Release()
{
	*m_pFileLog  << endl << "--------------FLUX ENGINE LOG END---------------" << endl;
	delete m_pFileLog;
	delete[] m_ConvertBuffer;
}

bool Console::LogFmodResult(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		stringstream stream;
		stream << "FMOD Error (" << result << ") " << FMOD_ErrorString(result);
		Log(stream.str(), LogType::ERROR);
		return true;
	}
	return false;
}

bool Console::LogHRESULT(const std::string &source, HRESULT hr)
{
	if (FAILED(hr))
	{
		if (FACILITY_WINDOWS == HRESULT_FACILITY(hr))
			hr = HRESULT_CODE(hr);

		std::stringstream ss;
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

bool Console::LogHRESULT(char* source, HRESULT hr)
{
	return LogHRESULT(string(source), hr);
}

void Console::Log(const std::string &message, LogType type)
{
	stringstream stream;
	stream << "[" << GetTime() << "]";
	switch (type)
	{
	case LogType::INFO:
		stream << "[INFO] ";
		break;
	case LogType::WARNING:
		SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		stream << "[WARNING] ";
		break;
	case LogType::ERROR:
		SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
		stream << "[ERROR] ";
		break;
	default:
		break;
	}

	stream << message;
	cout << stream.str() << endl;

	(*m_pFileLog) << stream.str() << endl;
	m_pFileLog->flush();
	SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

	if (type == LogType::ERROR)
	{
		MessageBox(nullptr, message.c_str(), "Fatal Error", MB_ICONINFORMATION);
		PostQuitMessage(-1);
	}
}

void Console::LogWarning(const std::string& message)
{
	Log(message, LogType::WARNING);
}

void Console::LogError(const std::string& message)
{
	Log(message, LogType::ERROR);
}

void Console::LogFormat(LogType type, const char* format, ...)
{
	va_list ap;

	va_start(ap, format);
	_vsnprintf_s(&m_ConvertBuffer[0], m_ConvertBufferSize, m_ConvertBufferSize, format, ap);
	va_end(ap);
	Log(&m_ConvertBuffer[0], type);
}

string Console::GetTime()
{
	time_t timer;
	time(&timer);
	tm localTime;
	localtime_s(&localTime, &timer);
	stringstream stream;
	stream << setfill('0');
	stream << setw(2) << localTime.tm_hour << ":" << setw(2) << localTime.tm_min << ":" << setw(2) << localTime.tm_sec;
	return stream.str();
}
