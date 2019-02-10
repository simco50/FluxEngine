#include "FluxEngine.h"
#include "FileSystem\File\PhysicalFile.h"
#include "Async\Thread.h"
#include "Core\Config.h"

static Console* consoleInstance = nullptr;

Console::Console()
{
	AUTOPROFILE(Console_Initialize);
	consoleInstance = this;

#ifdef _DEBUG
	InitializeConsoleWindow();
#endif

	m_ConvertBuffer = new char[m_ConvertBufferSize];

	if (Config::GetBool("CleanupLogs", "Console", true))
	{
		CleanupLogs(TimeSpan(Config::GetInt("LogRetention", "Console", 1) * Time::TicksPerDay));
	}

	TimeStamp time;
	DateTime::Now().Split(time);

	std::string filePath = Printf("%s\\%02d-%02d-%02d-%02d-%02d-%02d.log",
		Paths::LogsDir().c_str(),
		time.Year,
		time.Month,
		time.Day,
		time.Hour,
		time.Minute,
		time.Second);

	m_pFileLog = new PhysicalFile(filePath);

	if (!m_pFileLog->OpenWrite())
	{
		FLUX_LOG(Error, "Failed to open console log");
	}

	FLUX_LOG(Info, "Date: %02d-%02d-%02d", time.Day, time.Month, time.Year);
	FLUX_LOG(Info, "Time: %02d:%02d:%02d", time.Hour, time.Minute, time.Second);
	FLUX_LOG(Info, "Computer: %s | User: %s", Misc::ComputerName().c_str(), Misc::UserName().c_str());

	FLUX_LOG(Info, "Configuration: %s", BuildConfiguration::ToString(BuildConfiguration::Configuration));
	FLUX_LOG(Info, "Platform: %s", BuildPlatform::ToString(BuildPlatform::Platform));
	Misc::CpuId cpuId;
	Misc::GetCpuId(&cpuId);
	FLUX_LOG(Info, "Cpu: %s", cpuId.Brand.c_str());
	FLUX_LOG(Info, "Memory: %f MB", (float)Misc::GetTotalPhysicalMemory() / 1000.0f);
}

Console::~Console()
{
	delete m_pFileLog;
	delete[] m_ConvertBuffer;
}

void Console::FlushThreadedMessages()
{
	checkf(Thread::IsMainThread(), "Console::FlushThreadedMessages() must run on the main thread!");
	if (m_MessageQueue.size() == 0)
		return;

	ScopeLock lock(m_QueueMutex);
	while (m_MessageQueue.size() > 0)
	{
		const LogEntry& message = m_MessageQueue.front();
		Log(message.Message, message.Type);
		m_MessageQueue.pop();
	}
}

bool Console::LogHRESULT(const std::string &source, HRESULT hr)
{
	if (FAILED(hr))
	{
		if (FACILITY_WINDOWS == HRESULT_FACILITY(hr))
		{
			hr = HRESULT_CODE(hr);
		}

		std::stringstream ss;
		if (source.size() != 0)
		{
			ss << "Source: ";
			ss << source;
			ss << "\n";
		}
		ss << "Message: ";
		ss << Misc::GetErrorStringFromCode(hr);

		Log(ss.str(), LogType::Error);
		return true;
	}

	return false;
}

bool Console::LogHRESULT(char* source, HRESULT hr)
{
	return LogHRESULT(std::string(source), hr);
}

void Console::Log(const std::string &message, LogType type)
{
	if ((int)type < (int)consoleInstance->m_Verbosity)
	{
		return;
	}

	if (!Thread::IsMainThread())
	{
		ScopeLock lock(consoleInstance->m_QueueMutex);
		consoleInstance->m_MessageQueue.push(LogEntry(message, type));
	}
	else
	{
		std::stringstream stream;
		DateTime now = DateTime::Now();
		stream << "[" << now.GetHours() << ":" << now.GetMinutes() << ":" << now.GetSeconds() << "]";
		switch (type)
		{
		case LogType::Info:
			stream << "[INFO] ";
			break;
		case LogType::Warning:
			if (consoleInstance->m_ConsoleHandle)
				SetConsoleTextAttribute(consoleInstance->m_ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			stream << "[WARNING] ";
			break;
		case LogType::Error:
		case LogType::FatalError:
			if (consoleInstance->m_ConsoleHandle)
				SetConsoleTextAttribute(consoleInstance->m_ConsoleHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
			stream << "[ERROR] ";
			break;
		default:
			break;
		}

		stream << message;
		const std::string output = stream.str();
		std::cout << output << std::endl;
		if (consoleInstance->m_ConsoleHandle)
		{
			SetConsoleTextAttribute(consoleInstance->m_ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		}
		consoleInstance->m_pFileLog->WriteLine(output);

		consoleInstance->m_History.push_back(LogEntry(message, type));
		if (consoleInstance->m_History.size() > 50)
		{
			consoleInstance->m_History.pop_front();
		}

		if (type == LogType::Error)
		{
			__debugbreak();
		}
		else if (type == LogType::FatalError)
		{
			Misc::ShowMessageBox("Fatal Error", message);
			abort();
		}
	}
}

void Console::LogFormat(LogType type, const char* format, ...)
{
	va_list ap;

	va_start(ap, format);
	_vsnprintf_s(&consoleInstance->m_ConvertBuffer[0], consoleInstance->m_ConvertBufferSize, consoleInstance->m_ConvertBufferSize, format, ap);
	va_end(ap);
	Log(&consoleInstance->m_ConvertBuffer[0], type);
}

void Console::SetVerbosity(LogType type)
{
	consoleInstance->m_Verbosity = type;
}

bool Console::CleanupLogs(const TimeSpan& age)
{
	AUTOPROFILE(Console_CleanupLogs);

	struct LogCleaner : public FileVisitor
	{
		explicit LogCleaner(const TimeSpan& maxAge) :
			MaxAge(maxAge), Now(DateTime::Now())
		{
		}
		TimeSpan MaxAge;
		DateTime Now;

		virtual bool Visit(const std::string& fileName, const bool isDirectory) override
		{
			const std::string path = Paths::LogsDir() + fileName;
			if (isDirectory == false && Paths::GetFileExtenstion(fileName) == "log")
			{
				const TimeSpan age = Now - FileSystem::GetCreationTime(path);
				if (age > MaxAge)
				{
					FileSystem::Delete(path);
				}
			}
			return true;
		}

		virtual bool IsRecursive() const override
		{
			return false;
		}
	};
	LogCleaner cleaner(age);
	return FileSystem::IterateDirectory(Paths::LogsDir(), cleaner);
}

bool Console::Flush()
{
	checkf(Thread::IsMainThread(), "[Console::Flush()] Flush must be called from the main thread!");
	if (consoleInstance->m_pFileLog == nullptr)
	{
		return false;
	}
	return consoleInstance->m_pFileLog->Flush();
}

const std::deque<Console::LogEntry>& Console::GetHistory()
{
	return consoleInstance->m_History;
}

void Console::InitializeConsoleWindow()
{
	AUTOPROFILE(Console_InitializeConsoleWindow);

	if (AllocConsole())
	{
		// Redirect the CRT standard input, output, and error handles to the console
		FILE* pCout;
		errno_t e = freopen_s(&pCout, "CONIN$", "r", stdin);
		check(e == 0);
		e = freopen_s(&pCout, "CONOUT$", "w", stdout);
		check(e == 0);
		e = freopen_s(&pCout, "CONOUT$", "w", stderr);
		check(e == 0);

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

		//Set ConsoleHandle
		m_ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		//Disable Close-Button
		HWND hwnd = GetConsoleWindow();
		if (hwnd != nullptr)
		{
			HMENU hMenu = GetSystemMenu(hwnd, FALSE);
			if (hMenu != nullptr)
			{
				DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
			}
		}
	}
}