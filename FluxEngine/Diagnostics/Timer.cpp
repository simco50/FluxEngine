#include "FluxEngine.h"
#include "Timer.h"

Timer::Timer()
{
	Reset();
}

void Timer::Reset()
{
	QueryPerformanceFrequency(&m_Frequency);
	QueryPerformanceCounter(&m_StartTime);
}

float Timer::GetMilliSeconds() const
{
	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);
	return (float)((endTime.QuadPart - m_StartTime.QuadPart) * 1000.0 / m_Frequency.QuadPart);
}
