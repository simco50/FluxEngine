#pragma once
#include "TimeSpan.h"

struct DateTime
{
	DateTime() :
		Ticks(0)
	{}

	DateTime(unsigned long long ticks) :
		Ticks(ticks)
	{}

	DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond)
	{
		int days = 0;
		if (month > 2 && year % 4 == 0)
		{
			if (year % 100 != 0 || year % 400 == 0)
			{
				++days;
			}
		}
		month--;
		year--;

		days += year * 365;
		days += year / 4;
		days -= year / 100;
		days += year / 400;
		days += Time::DaysToMonth[month];
		days += day - 1;

		Ticks = days * Time::TicksPerDay
			+ hour * Time::TicksPerHour
			+ minute * Time::TicksPerMinute
			+ second * Time::TicksPerSecond
			+ millisecond * Time::TicksPerMillisecond;
	}

	void Split(TimeStamp& timestamp)
	{
		GetDate(timestamp.Year, timestamp.Month, timestamp.Day);
		timestamp.Hour = GetHours();
		timestamp.Minute = GetMinutes();
		timestamp.Second = GetSeconds();
		timestamp.Millisecond = GetMilliSeconds();
	}

	void GetDate(int& year, int& month, int& day) const
	{
		// Based on FORTRAN code in:
		// Fliegel, H. F. and van Flandern, T. C.,
		// Communications of the ACM, Vol. 11, No. 10 (October 1968).

		int i, j, k, l, n;

		l = (int)floor(GetJulianDay() + 0.5) + 68569;
		n = 4 * l / 146097;
		l = l - (146097 * n + 3) / 4;
		i = 4000 * (l + 1) / 1461001;
		l = l - 1461 * i / 4 + 31;
		j = 80 * l / 2447;
		k = l - 2447 * j / 80;
		l = j / 11;
		j = j + 2 - 12 * l;
		i = 100 * (n - 49) + i + l;

		year = i;
		month = j;
		day = k;
	}

	static DateTime Now()
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		return DateTime(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	}

	static int UtcNow()
	{
		return (int)((DateTime::Now() - DateTime(1970, 1, 1, 0, 0, 0, 0)).Ticks / Time::TicksPerSecond);
	}

	int GetMilliSeconds() const { return Ticks / Time::TicksPerMillisecond % 1000; }
	int GetSeconds() const { return Ticks / Time::TicksPerSecond % 60; }
	int GetMinutes() const { return Ticks / Time::TicksPerMinute % 60; }
	int GetHours() const { return (int)(Ticks / Time::TicksPerHour % 24); }
	int GetDay() const
	{
		int year, month, day;
		GetDate(year, month, day);
		return day;
	}
	int GetMonth() const
	{
		int year, month, day;
		GetDate(year, month, day);
		return month;
	}
	int GetYear() const
	{
		int year, month, day;
		GetDate(year, month, day);
		return year;
	}

	std::string ToString() const
	{
		std::stringstream str;
		int year, month, day;
		GetDate(year, month, day);
		str << year << "/" << month << "/" << day << " " << GetHours() << ":" << GetMinutes() << ":" << GetSeconds();
		return str.str();
	}

	bool operator==(const DateTime& other) const { return Ticks == other.Ticks; }
	bool operator!=(const DateTime& other) const { return Ticks != other.Ticks; }
	bool operator<(const DateTime& other) const { return Ticks < other.Ticks; }
	bool operator>(const DateTime& other) const { return Ticks > other.Ticks; }
	bool operator<=(const DateTime& other) const { return Ticks <= other.Ticks; }
	bool operator>=(const DateTime& other) const { return Ticks >= other.Ticks; }

	TimeSpan operator+(const DateTime& other) const { return TimeSpan(Ticks + other.Ticks); }
	TimeSpan operator-(const DateTime& other) const { return TimeSpan(Ticks - other.Ticks); }

	unsigned long long Ticks;

private:

	double GetJulianDay() const
	{
		return (double)(1721425.5 + Ticks / Time::TicksPerDay);
	}
};