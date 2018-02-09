#pragma once
#include "Helpers/Singleton.h"

class Profiler : public Singleton<Profiler>
{
public:
	Profiler();
	~Profiler();

	void OutputLog(IFile* pFile, int maxDepth = 20);

	struct AutoProfilerBlock
	{
		AutoProfilerBlock(const std::string& name, const std::string& description, AutoProfilerBlock* pParent) :
			Name(name), Description(description), pParent(pParent)
		{
			QueryPerformanceCounter((LARGE_INTEGER*)&BeginTime);
		}

		std::string Name;
		std::string Description;
		AutoProfilerBlock* pParent;
		int Frame = -1;
		__int64 BeginTime;
		std::queue<std::unique_ptr<AutoProfilerBlock>> Children;
		double Time = 0.0;
	};

	void BeginBlock(const std::string& name, const std::string& description = "");

	void EndBlock();

	std::unique_ptr<AutoProfilerBlock> m_pRootBlock;
	AutoProfilerBlock* m_pCurrentBlock = nullptr;
	__int64 m_Frequency;
};

class AutoProfiler
{
public:
	AutoProfiler(const std::string& name, const std::string& description = "")
	{
		Profiler::Instance()->BeginBlock(name, description);
	}
	~AutoProfiler()
	{
		Profiler::Instance()->EndBlock();
	}
};

#define AUTOPROFILE(name) AutoProfiler Profiler_##name(#name)

#define AUTOPROFILE_DESC(name, desc) AutoProfiler Profiler_##name(#name, desc)