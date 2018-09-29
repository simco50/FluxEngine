#include "FluxEngine.h"
#include "TimeSpan.h"

TimeSpan::TimeSpan() :
	m_Ticks(0)
{

}

TimeSpan::TimeSpan(unsigned long long ticks) :
	m_Ticks(ticks)
{

}

TimeSpan& TimeSpan::operator-=(const TimeSpan& other)
{
	m_Ticks -= other.m_Ticks;
	return *this;
}

TimeSpan TimeSpan::operator-(const TimeSpan& other) const
{
	return TimeSpan(m_Ticks - other.m_Ticks);
}

bool TimeSpan::operator>=(TimeSpan& other) const
{
	return m_Ticks >= other.m_Ticks;
}

bool TimeSpan::operator>(TimeSpan& other) const
{
	return m_Ticks > other.m_Ticks;
}

bool TimeSpan::operator!=(TimeSpan& other) const
{
	return m_Ticks != other.m_Ticks;
}

bool TimeSpan::operator==(TimeSpan& other) const
{
	return m_Ticks == other.m_Ticks;
}

int TimeSpan::GetMilliSeconds() const
{
	return m_Ticks / Time::TicksPerMillisecond % 1000;
}

int TimeSpan::GetSeconds() const
{
	return m_Ticks / Time::TicksPerSecond % 60;
}

int TimeSpan::GetMinutes() const
{
	return m_Ticks / Time::TicksPerMinute % 60;
}

int TimeSpan::GetHours() const
{
	return (int)(m_Ticks / Time::TicksPerHour % 24);
}

int TimeSpan::GetDays() const
{
	return (int)(m_Ticks / Time::TicksPerDay);
}

double TimeSpan::GetTotalMinutes() const
{
	return (double)m_Ticks / Time::TicksPerMinute;
}

double TimeSpan::GetTotalSeconds() const
{
	return (double)m_Ticks / Time::TicksPerSecond;
}

double TimeSpan::GetTotalMilliseconds() const
{
	return (double)m_Ticks / Time::TicksPerMillisecond;
}

double TimeSpan::GetTotalMicroseconds() const
{
	return (double)m_Ticks / Time::TicksPerMicrosecond;
}

bool TimeSpan::operator<=(TimeSpan& other) const
{
	return m_Ticks <= other.m_Ticks;
}

bool TimeSpan::operator<(TimeSpan& other) const
{
	return m_Ticks < other.m_Ticks;
}

TimeSpan& TimeSpan::operator+=(const TimeSpan& other)
{
	m_Ticks += other.m_Ticks;
	return *this;
}

TimeSpan TimeSpan::operator+(const TimeSpan& other) const
{
	return TimeSpan(m_Ticks + other.m_Ticks);
}
