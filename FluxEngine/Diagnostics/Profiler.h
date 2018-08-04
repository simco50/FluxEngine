#pragma once
#include "Helpers/Singleton.h"

class Profiler : public Singleton<Profiler>
{
public:
	Profiler();
	~Profiler();

	void OutputLog(File* pFile, int maxDepth = 20);

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
		int64 BeginTime;
		std::deque<std::unique_ptr<AutoProfilerBlock>> Children;
		double Time = 0.0;
	};

	void BeginBlock(const std::string& name, const std::string& description = "");
	void EndBlock();

private:
	bool IsFinalized(std::string& lastBlock);

	std::unique_ptr<AutoProfilerBlock> m_pRootBlock;
	AutoProfilerBlock* m_pCurrentBlock = nullptr;
	int64 m_Frequency;
	Mutex m_BlockMutex;
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

#define PROFILE_START(name) Profiler::Instance()->BeginBlock(#name, "")
#define PROFILE_END() Profiler::Instance()->EndBlock()