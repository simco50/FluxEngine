#include "stdafx.h"
#include <time.h>
#include <iomanip>
#include "FileSystem\File\PhysicalFile.h"

using namespace std;

IFile* Console::m_pFileLog = nullptr;
HANDLE Console::m_ConsoleHandle;
char* Console::m_ConvertBuffer = new char[m_ConvertBufferSize];

Console::Console()
{}

Console::~Console()
{
}

void Console::Initialize()
{
	time_t timer;
	time(&timer);
	tm localTime;
	localtime_s(&localTime, &timer);
	stringstream filePathStream;
	filePathStream << ".\\Logs\\" << 1900 + localTime.tm_year << "-" << localTime.tm_mon + 1 << "-" << localTime.tm_mday << "_" << localTime.tm_hour << "-" << localTime.tm_min << "-" << localTime.tm_sec << ".log";

	m_pFileLog = new PhysicalFile(filePathStream.str());

	if (!m_pFileLog->Open(FileMode::Write, ContentType::Text))
	{
		FLUX_LOG(ERROR, "Failed to open console log");
	}

	*m_pFileLog << IFile::endl << "-------------FLUX ENGINE LOG START--------------" << IFile::endl << IFile::endl;

	*m_pFileLog << "Date: " << localTime.tm_mday << "-" << localTime.tm_mon + 1 << "-" << 1900 + localTime.tm_year << IFile::endl;
	*m_pFileLog << "Time: " << GetTime() << IFile::endl;

#ifdef _DEBUG
	InitializeConsoleWindow();
#endif
}

void Console::Release()
{
	*m_pFileLog  << IFile::endl << "--------------FLUX ENGINE LOG END---------------" << IFile::endl;
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
		if(m_ConsoleHandle)
			SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		stream << "[WARNING] ";
		break;
	case LogType::ERROR:
		if (m_ConsoleHandle)
			SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
		stream << "[ERROR] ";
		break;
	default:
		break;
	}

	stream << message;
	if (m_ConsoleHandle)
	{
		cout << stream.str() << endl;
		SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	}

	*m_pFileLog << stream.str() << IFile::endl;

	if (type == LogType::ERROR)
	{
		MessageBox(nullptr, message.c_str(), "Fatal Error", MB_ICONINFORMATION);
		//PostQuitMessage(-1);
		//__debugbreak();
		abort();
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

void Console::LogFormat(LogType type, const std::string& format, ...)
{
	va_list ap;

	const char* f = format.c_str();
	va_start(ap, f);
	_vsnprintf_s(&m_ConvertBuffer[0], m_ConvertBufferSize, m_ConvertBufferSize, f, ap);
	va_end(ap);
	Log(&m_ConvertBuffer[0], type);
}

void Console::InitializeConsoleWindow()
{
	if (AllocConsole())
	{
		// Redirect the CRT standard input, output, and error handles to the console
		FILE* pCout;
		freopen_s(&pCout, "CONIN$", "r", stdin);
		freopen_s(&pCout, "CONOUT$", "w", stdout);
		freopen_s(&pCout, "CONOUT$", "w", stderr);

		//Clear the error state for each of the C++ standard stream objects. We need to do this, as
		//attempts to access the standard streams before they refer to a valid target will cause the
		//iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
		//to always occur during startup regardless of whether anything has been read from or written to
		//the console or not.
		std::wcout.clear();
		std::cout.clear();
		std::wcerr.clear();
		std::cerr.clear();
		std::wcin.clear();
		std::cin.clear();
		std::cin.clear();

		//Set ConsoleHandle
		m_ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		//Disable Close-Button
		HWND hwnd = GetConsoleWindow();
		if (hwnd != NULL)
		{
			HMENU hMenu = GetSystemMenu(hwnd, FALSE);
			if (hMenu != NULL) DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
		}
	}
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
