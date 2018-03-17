#pragma once

class Timer
{
public:
	Timer();
	void Reset();
	float GetMilliSeconds() const;

private:
	LARGE_INTEGER m_StartTime, m_Frequency;
};