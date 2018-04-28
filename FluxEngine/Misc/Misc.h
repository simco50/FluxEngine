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

	std::string ToString(Type type);
	Type FromString(const std::string& configuration);
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

	std::string ToString(Type type);
	Type FromString(const std::string& configuration);
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
};