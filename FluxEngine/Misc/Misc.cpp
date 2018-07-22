#include "FluxEngine.h"
#include "Misc.h"
#include <shellapi.h>

namespace BuildConfiguration
{
#ifdef _DEBUG
	Type Configuration = Type::Debug;
#elif defined(NDEBUG)
	Type Configuration = Type::Release;
#else
	Type Configuration = Type::Other;
#endif

	std::string ToString(Type type)
	{
		switch (type)
		{
		case Type::Debug:
			return "Debug";
		case Type::Release:
			return "Release";
		default:
			return "Other";
		}
	}

	Type FromString(const std::string& configuration)
	{
		if (configuration == "Debug")
		{
			return Type::Debug;
		}
		else if (configuration == "Release")
		{
			return Type::Release;
		}
		return Type::Other;
	}
}

namespace BuildPlatform
{
#ifdef x64
	Type Platform = Type::X64;
#elif defined(x86)
	Type Platform = Type::X86;
#else
	Type Platform = Type::Other;
#endif

	std::string ToString(Type type)
	{
		switch (type)
		{
		case Type::X64:
			return "x64";
		case Type::X86:
			return "x86";
		default:
			return "Other";
		}
	}

	Type FromString(const std::string& configuration)
	{
		if (configuration == "x64")
		{
			return Type::X64;
		}
		else if (configuration == "x86")
		{
			return Type::X86;
		}
		return Type::Other;
	}

}

bool Misc::SetEnvironmentVar(const std::string& name, const std::string& value)
{
	uint32 error = ::SetEnvironmentVariable(name.c_str(), value.c_str());
	return error != 0;
}

bool Misc::GetEnvironmentVar(const std::string& name, std::string& value)
{
	char buffer[128];
	DWORD size = 0;
	uint32 error = ::GetEnvironmentVariableA(name.c_str(), buffer, size);
	if (error <= 0)
	{
		value = "";
		return false;
	}
	value = buffer;
	return true;
}

bool Misc::GetDiskSpace(char driveLetter, uint64& totalSpace, uint64& freeSpace)
{
	std::string drive = driveLetter + ":\\";
	return ::GetDiskFreeSpaceEx(drive.c_str(), nullptr, reinterpret_cast<ULARGE_INTEGER*>(&totalSpace), reinterpret_cast<ULARGE_INTEGER*>(&freeSpace));
}

int Misc::GetCoreCount()
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	return sysInfo.dwNumberOfProcessors;
}

uint64 Misc::GetTotalPhysicalMemory()
{
	uint64 memory;
	GetPhysicallyInstalledSystemMemory(&memory);
	return memory;
}

void Misc::GetCpuId(CpuId* pCpuId)
{
	std::array<int, 4> cpuId = {};
	__cpuid(cpuId.data(), 0);
	int nIds = cpuId[0];

	std::vector<std::array<int, 4>> data;
	for (int i = 0; i <= nIds; i++)
	{
		__cpuidex(cpuId.data(), i, 0);
		data.push_back(cpuId);
	}
	char vendor[0x20];
	memset(vendor, 0, sizeof(vendor));
	*reinterpret_cast<int*>(vendor) = data[0][1];
	*reinterpret_cast<int*>(vendor + 4) = data[0][3];
	*reinterpret_cast<int*>(vendor + 8) = data[0][2];
	pCpuId->Vendor = vendor;
	if (pCpuId->Vendor == "GenuineIntel")
	{
		pCpuId->Type = CpuId::Type::Intel;
	}
	else if (pCpuId->Vendor == "AuthenticAMD")
	{
		pCpuId->Type = CpuId::Type::AMD;
	}
	else
	{
		pCpuId->Type = CpuId::Type::Other;
	}

	__cpuid(cpuId.data(), 0x80000000);
	int nExIds = cpuId[0];

	std::vector<std::array<int, 4>> extdata;
	for (int i = 0x80000000; i <= nExIds; i++)
	{
		__cpuidex(cpuId.data(), i, 0);
		extdata.push_back(cpuId);
	}

	char brand[0x40];
	memset(brand, 0, sizeof(brand));
	if (nExIds >= 0x80000004)
	{
		memcpy(brand, extdata[2].data(), sizeof(cpuId));
		memcpy(brand + 16, extdata[3].data(), sizeof(cpuId));
		memcpy(brand + 32, extdata[4].data(), sizeof(cpuId));
		pCpuId->Brand = brand;
	}

	__cpuid(cpuId.data(), 0x80000006);
	pCpuId->CacheLineSize = cpuId[2] & 0xFF;
}

bool Misc::MessageBox(const std::string& title, const std::string& description)
{
	::MessageBox(nullptr,description.c_str(), title.c_str(), MB_OK | MB_SYSTEMMODAL);
	return true;
}

bool Misc::OsExecute(const std::string& command, const std::string& commandLine, const std::string& operation /*= ""*/)
{
	HINSTANCE process = ShellExecute(0,
		operation.c_str(),
		command.c_str(),
		commandLine.c_str(),
		0,
		SW_SHOWNORMAL);
	return process > (HINSTANCE)32;
}

int Misc::GetLastErrorCode()
{
	return ::GetLastError();
}

int Misc::GetLastErrorCodeHR()
{
	return HRESULT_FROM_WIN32(GetLastErrorCode());
}

std::string Misc::GetErrorStringFromCode(int errorCode)
{
	TCHAR* errorMsg;
	std::string output;
	if (FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&errorMsg, 0, nullptr) != 0)
	{
		output = errorMsg;
	}
	else
	{
		output = "Unknown Error";
	}
	return output;
}

uint32 Misc::GetProcessId()
{
	return ::GetCurrentProcessId();
}

std::string Misc::GetComputerName()
{
	char buffer[256];
	uint32 size = sizeof(buffer);
	::GetComputerName(buffer, (DWORD*)&size);
	return buffer;
}

std::string Misc::GetUserName()
{
	char buffer[256];
	uint32 size = sizeof(buffer);
	::GetUserName(buffer, (DWORD*)&size);
	return buffer;
}