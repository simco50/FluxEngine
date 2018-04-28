#pragma once
#include <time.h>

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
		Ticks(0)
	{}

	TimeSpan(unsigned long long ticks) :
		Ticks(ticks)
	{}

	int GetMilliSeconds() const { return Ticks / Time::TicksPerMillisecond % 1000; }
	int GetSeconds() const { return Ticks / Time::TicksPerSecond % 60; }
	int GetMinutes() const { return Ticks / Time::TicksPerMinute % 60; }
	int GetHours() const { return (int)(Ticks / Time::TicksPerHour % 24); }
	int GetDays() const { return (int)(Ticks / Time::TicksPerDay); }

	double GetTotalMinutes() const { return (double)Ticks / Time::TicksPerMinute; }
	double GetTotalSeconds() const { return (double)Ticks / Time::TicksPerSecond; }
	double GetTotalMilliseconds() const { return (double)Ticks / Time::TicksPerMillisecond; }
	double GetTotalMicroseconds() const { return (double)Ticks / Time::TicksPerMicrosecond; }

	bool operator==(TimeSpan& other) const { return Ticks == other.Ticks; }
	bool operator!=(TimeSpan& other) const { return Ticks != other.Ticks; }
	bool operator<(TimeSpan& other) const { return Ticks < other.Ticks; }
	bool operator>(TimeSpan& other) const { return Ticks > other.Ticks; }
	bool operator<=(TimeSpan& other) const { return Ticks <= other.Ticks; }
	bool operator>=(TimeSpan& other) const { return Ticks >= other.Ticks; }

	TimeSpan operator+(const TimeSpan& other) const { return TimeSpan(Ticks + other.Ticks); }
	TimeSpan operator-(const TimeSpan& other) const { return TimeSpan(Ticks - other.Ticks); }
	
	TimeSpan& operator+=(const TimeSpan& other) 
	{ 
		Ticks += other.Ticks;
		return *this;
	}

	TimeSpan& operator-=(const TimeSpan& other) 
	{
		Ticks -= other.Ticks; 
		return *this;
	}

	unsigned long long Ticks;
};