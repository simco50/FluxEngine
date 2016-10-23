#include "stdafx.h"
#include "GameTimer.h"


GameTimer::GameTimer()
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_SecondsPerCount = 1.0f / (double)(countsPerSec);
}

GameTimer::~GameTimer()
{
}

void GameTimer::Tick()
{
	if(m_IsStopped)
	{
		m_DeltaTime = 0.0f;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_CurrTime = currTime;

	m_DeltaTime = (m_CurrTime - m_PrevTime) * m_SecondsPerCount;

	m_PrevTime = m_CurrTime;

	if (m_DeltaTime < 0.0f)
		m_DeltaTime = 0.0f;
}

float GameTimer::GameTime() const
{
	if(m_IsStopped)
		return (float)(((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	return (float)(((m_CurrTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
}

float GameTimer::DeltaTime() const
{
	return (float)m_DeltaTime;
}

void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_BaseTime = currTime;
	m_PrevTime = currTime;
	m_StopTime = 0;
	m_IsStopped = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if(m_IsStopped)
	{
		m_PausedTime += (startTime - m_StopTime);
		m_PrevTime = startTime;

		m_StopTime = 0;
		m_IsStopped = false;
	}
}

void GameTimer::Stop()
{
	if(!m_IsStopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		m_StopTime = currTime;
		m_IsStopped = true;
	}
}