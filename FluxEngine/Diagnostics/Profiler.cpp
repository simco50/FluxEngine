#include "FluxEngine.h"
#include "Profiler.h"
#include "FileSystem\File\PhysicalFile.h"

Profiler::Profiler() :
	m_pRootBlock(std::make_unique<AutoProfilerBlock>("Root", "", nullptr))
{
	m_pCurrentBlock = m_pRootBlock.get();

	QueryPerformanceFrequency((LARGE_INTEGER*)&m_Frequency);
}

Profiler::~Profiler()
{
	__int64 endTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&endTime);
	m_pRootBlock->Time = (endTime - m_pCurrentBlock->BeginTime) * 1000.0 / m_Frequency;

	std::string lastBlock;
	if (IsFinalized(lastBlock) == false)
	{
		FLUX_LOG(Error, "[Profiler::~Profiler()] > Profiler is not properly closed. Ended with '%s'", lastBlock.c_str());
	}

	std::unique_ptr<PhysicalFile> pFile = std::make_unique<PhysicalFile>(Paths::ProfilingDir() + "\\Profiler.log");
	pFile->OpenWrite();
	OutputLog(pFile.get());
	pFile->Close();
}

void Profiler::OutputLog(File* pFile, int maxDepth)
{
	m_pCurrentBlock = m_pRootBlock->Children.front().get();
	int depth = 0;
	for (;;)
	{
		if (depth <= maxDepth)
		{
			std::stringstream stream;
			stream << "[" << m_pCurrentBlock->Frame << "]\t";
			for (int i = 0; i < depth; ++i)
			{
				stream << "\t";
			}
			if (m_pCurrentBlock->Description.empty())
				stream << "[" << m_pCurrentBlock->Name << "] > " << m_pCurrentBlock->Time << " ms" << std::endl;
			else
				stream << "[" << m_pCurrentBlock->Name << "] > " << m_pCurrentBlock->Description << " : " << m_pCurrentBlock->Time << " ms" << std::endl;
			std::string output = stream.str();
			pFile->Write(output.c_str(), output.size());
		}

		while (m_pCurrentBlock->Children.size() == 0)
		{
			m_pCurrentBlock = m_pCurrentBlock->pParent;
			if (m_pCurrentBlock == nullptr)
				return;
			m_pCurrentBlock->Children.pop_front();
			--depth;
		}
		m_pCurrentBlock = m_pCurrentBlock->Children.front().get();
		depth++;
	}
}

void Profiler::BeginBlock(const std::string& name, const std::string& description)
{
	std::unique_ptr<AutoProfilerBlock> pBlock = std::make_unique<AutoProfilerBlock>(name, description, m_pCurrentBlock);
	pBlock->Frame = GameTimer::Ticks();
	AutoProfilerBlock* pNewBlock = pBlock.get();
	m_pCurrentBlock->Children.push_back(std::move(pBlock));
	m_pCurrentBlock = pNewBlock;
}

void Profiler::EndBlock()
{
	__int64 endTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&endTime);
	m_pCurrentBlock->Time = (endTime - m_pCurrentBlock->BeginTime) * 1000.0f / m_Frequency;

	m_pCurrentBlock = m_pCurrentBlock->pParent;
}

bool Profiler::IsFinalized(std::string& lastBlock)
{
	if (m_pCurrentBlock)
	{
		lastBlock = m_pCurrentBlock->Name;
		return lastBlock == "Root";
	}
	return true;
}