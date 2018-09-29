#pragma once
namespace Time
{
	const unsigned long long TicksPerDay = 864000000000;
	const unsigned long long TicksPerHour = 36000000000;
	const unsigned long long TicksPerMicrosecond = 10;
	const unsigned long long TicksPerMillisecond = 10000;
	const unsigned long long TicksPerMinute = 600000000;
	const unsigned long long TicksPerSecond = 10000000;
	const unsigned long long TicksPerWeek = 6048000000000;

	const int DaysToMonth[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
}

struct TimeStamp
{
	int Year = 0;
	int Month = 0;
	int Day = 0;
	int Hour = 0;
	int Minute = 0;
	int Second = 0;
	int Millisecond = 0;
};

struct TimeSpan
{
	TimeSpan() :
		m_Ticks(0)
	{}

	TimeSpan(unsigned long long ticks) :
		m_Ticks(ticks)
	{}

	int GetMilliSeconds() const { return m_Ticks / Time::TicksPerMillisecond % 1000; }
	int GetSeconds() const { return m_Ticks / Time::TicksPerSecond % 60; }
	int GetMinutes() const { return m_Ticks / Time::TicksPerMinute % 60; }
	int GetHours() const { return (int)(m_Ticks / Time::TicksPerHour % 24); }
	int GetDays() const { return (int)(m_Ticks / Time::TicksPerDay); }

	double GetTotalMinutes() const { return (double)m_Ticks / Time::TicksPerMinute; }
	double GetTotalSeconds() const { return (double)m_Ticks / Time::TicksPerSecond; }
	double GetTotalMilliseconds() const { return (double)m_Ticks / Time::TicksPerMillisecond; }
	double GetTotalMicroseconds() const { return (double)m_Ticks / Time::TicksPerMicrosecond; }

	bool operator==(TimeSpan& other) const { return m_Ticks == other.m_Ticks; }
	bool operator!=(TimeSpan& other) const { return m_Ticks != other.m_Ticks; }
	bool operator<(TimeSpan& other) const { return m_Ticks < other.m_Ticks; }
	bool operator>(TimeSpan& other) const { return m_Ticks > other.m_Ticks; }
	bool operator<=(TimeSpan& other) const { return m_Ticks <= other.m_Ticks; }
	bool operator>=(TimeSpan& other) const { return m_Ticks >= other.m_Ticks; }

	TimeSpan operator+(const TimeSpan& other) const { return TimeSpan(m_Ticks + other.m_Ticks); }
	TimeSpan operator-(const TimeSpan& other) const { return TimeSpan(m_Ticks - other.m_Ticks); }

	TimeSpan& operator+=(const TimeSpan& other)
	{
		m_Ticks += other.m_Ticks;
		return *this;
	}

	TimeSpan& operator-=(const TimeSpan& other)
	{
		m_Ticks -= other.m_Ticks;
		return *this;
	}

	unsigned long long m_Ticks;
};