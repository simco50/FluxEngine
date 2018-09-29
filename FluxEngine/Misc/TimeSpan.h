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
	TimeSpan();

	TimeSpan(unsigned long long ticks);

	int GetMilliSeconds() const;
	int GetSeconds() const;
	int GetMinutes() const;
	int GetHours() const;
	int GetDays() const;

	double GetTotalMinutes() const;
	double GetTotalSeconds() const;
	double GetTotalMilliseconds() const;
	double GetTotalMicroseconds() const;

	bool operator==(TimeSpan& other) const;
	bool operator!=(TimeSpan& other) const;
	bool operator<(TimeSpan& other) const;
	bool operator>(TimeSpan& other) const;
	bool operator<=(TimeSpan& other) const;
	bool operator>=(TimeSpan& other) const;

	TimeSpan operator+(const TimeSpan& other) const;
	TimeSpan operator-(const TimeSpan& other) const;

	TimeSpan& operator+=(const TimeSpan& other);

	TimeSpan& operator-=(const TimeSpan& other);

	unsigned long long m_Ticks;
};