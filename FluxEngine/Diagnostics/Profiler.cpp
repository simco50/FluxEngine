#include "stdafx.h"
#include "Profiler.h"
#include "FileSystem\File\PhysicalFile.h"

Profiler::Profiler()
{
	m_pRootBlock = std::make_unique<AutoProfilerBlock>("Root", "", nullptr);
	m_pCurrentBlock = m_pRootBlock.get();

	QueryPerformanceFrequency((LARGE_INTEGER*)&m_Frequency);
}

Profiler::~Profiler()
{
	__int64 endTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&endTime);
	m_pRootBlock->Time = (endTime - m_pCurrentBlock->BeginTime) * 1000.0 / m_Frequency;

	unique_ptr<PhysicalFile> pFile = make_unique<PhysicalFile>("Profiler.log");
	pFile->Open(FileMode::Write, ContentType::Text);
	OutputLog(pFile.get());
	pFile->Close();
}

void Profiler::OutputLog(IFile* pFile, int maxDepth)
{
	m_pCurrentBlock = m_pRootBlock->Children.front().get();
	int depth = 0;
	for (;;)
	{
		if (depth <= maxDepth)
		{
			*pFile << "[" << m_pCurrentBlock->Frame << "]\t";
			for (int i = 0; i < depth; ++i)
			{
				*pFile << "\t";
			}
			if (m_pCurrentBlock->Description.empty())
				*pFile << "[" << m_pCurrentBlock->Name << "] > " << m_pCurrentBlock->Time << " ms" << IFile::endl;
			else
				*pFile << "[" << m_pCurrentBlock->Name << "] > " << m_pCurrentBlock->Description << " : " << m_pCurrentBlock->Time << " ms" << IFile::endl;
		}

		while (m_pCurrentBlock->Children.size() == 0)
		{
			m_pCurrentBlock = m_pCurrentBlock->pParent;
			if (m_pCurrentBlock == nullptr)
				return;
			m_pCurrentBlock->Children.pop();
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
	m_pCurrentBlock->Children.push(std::move(pBlock));
	m_pCurrentBlock = pNewBlock;
}

void Profiler::EndBlock()
{
	__int64 endTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&endTime);
	m_pCurrentBlock->Time = (endTime - m_pCurrentBlock->BeginTime) * 1000.0f / m_Frequency;
	m_pCurrentBlock = m_pCurrentBlock->pParent;
}
