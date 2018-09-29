#pragma once

namespace BuildConfiguration
{
	enum Type
	{
		Debug,
		Release,
		Other
	};

	extern Type Configuration;

	const char* ToString(Type type);
	Type FromString(const char* configuration);
}

namespace BuildPlatform
{
	enum Type
	{
		X86,
		X64,
		Other,
	};

	extern Type Platform;

	const char* ToString(Type type);
	Type FromString(const char* configuration);
}

struct Misc
{
	static bool SetEnvironmentVar(const std::string& name, const std::string& value);
	static bool GetEnvironmentVar(const std::string& name, std::string& value);

	static bool GetDiskSpace(char driveLetter, uint64& totalSpace, uint64& freeSpace);
	static int GetCoreCount();
	static uint64 GetTotalPhysicalMemory();

	struct CpuId
	{
		std::string Vendor;
		std::string Brand;
		enum class Type
		{
			Intel,
			AMD,
			Other
		} Type;
		int CacheLineSize = 0;
	};

	static void GetCpuId(CpuId* pCpuId);

	static bool MessageBox(const std::string& title, const std::string& description);
	static bool OsExecute(const std::string& command, const std::string& commandLine, const std::string& operation = "open");

	static int GetLastErrorCode();
	static int GetLastErrorCodeHR();
	static std::string GetErrorStringFromCode(int errorCode);

	static uint32 GetProcessId();
	static std::string GetComputerName();
	static std::string GetUserName();
};