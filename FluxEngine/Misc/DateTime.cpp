#include "FluxEngine.h"
#include "DateTime.h"

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond)
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

	m_Ticks = days * Time::TicksPerDay
		+ hour * Time::TicksPerHour
		+ minute * Time::TicksPerMinute
		+ second * Time::TicksPerSecond
		+ millisecond * Time::TicksPerMillisecond;
}

DateTime::DateTime(unsigned long long ticks) :
	m_Ticks(ticks)
{

}

DateTime::DateTime() :
	m_Ticks(0)
{

}

bool DateTime::operator>=(const DateTime& other) const
{
	return m_Ticks >= other.m_Ticks;
}

bool DateTime::operator>(const DateTime& other) const
{
	return m_Ticks > other.m_Ticks;
}

bool DateTime::operator!=(const DateTime& other) const
{
	return m_Ticks != other.m_Ticks;
}

bool DateTime::operator==(const DateTime& other) const
{
	return m_Ticks == other.m_Ticks;
}

std::string DateTime::ToDateString() const
{
	int year, month, day;
	GetDate(year, month, day);
	char buffer[sizeof("0000-00-00")];
	sprintf_s(buffer, "%04d-%02d-%02d", year, month, day);
	return std::string(buffer);
}

int DateTime::UtcNow()
{
	return (int)((DateTime::Now() - DateTime(1970, 1, 1, 0, 0, 0, 0)).m_Ticks / Time::TicksPerSecond);
}

int DateTime::GetMilliSeconds() const
{
	return m_Ticks / Time::TicksPerMillisecond % 1000;
}

int DateTime::GetSeconds() const
{
	return m_Ticks / Time::TicksPerSecond % 60;
}

int DateTime::GetMinutes() const
{
	return m_Ticks / Time::TicksPerMinute % 60;
}

int DateTime::GetHours() const
{
	return (int)(m_Ticks / Time::TicksPerHour % 24);
}

int DateTime::GetHours12() const
{
	int hours = GetHours();
	if (hours > 12)
	{
		hours -= 12;
	}
	return hours;
}

int DateTime::GetDay() const
{
	int year, month, day;
	GetDate(year, month, day);
	return day;
}

int DateTime::GetMonth() const
{
	int year, month, day;
	GetDate(year, month, day);
	return month;
}

int DateTime::GetYear() const
{
	int year, month, day;
	GetDate(year, month, day);
	return year;
}

bool DateTime::IsMorning() const
{
	return GetHours() < 12;
}

void DateTime::Split(TimeStamp& timestamp) const
{
	GetDate(timestamp.Year, timestamp.Month, timestamp.Day);
	timestamp.Hour = GetHours();
	timestamp.Minute = GetMinutes();
	timestamp.Second = GetSeconds();
	timestamp.Millisecond = GetMilliSeconds();
}

void DateTime::GetDate(int& year, int& month, int& day) const
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

DateTime DateTime::Now()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	return DateTime(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

std::string DateTime::ToString() const
{
	std::stringstream str;
	int year, month, day;
	GetDate(year, month, day);
	str << year << "/" << month << "/" << day << " " << GetHours() << ":" << GetMinutes() << ":" << GetSeconds();
	return str.str();
}

std::string DateTime::ToString(const char* pFormat) const
{
	std::stringstream stream;

	int year, month, day;
	GetDate(year, month, day);

	while (*pFormat != '\0')
	{
		if (*pFormat == '%' && *(++pFormat) != '\0')
		{
			switch (*pFormat)
			{
			case 'a': stream << (IsMorning() ? "am" : "pm"); break;
			case 'A': stream << (IsMorning() ? "AM" : "PM"); break;
			case 'H': stream << Printf("%02d", GetHours()); break;
			case 'M': stream << Printf("%02d", GetMinutes()); break;
			case 'S': stream << Printf("%02d", GetSeconds()); break;
			case 's': stream << Printf("%02d", GetMilliSeconds()); break;
			case 'd': stream << Printf("%02d", day); break;
			case 'm': stream << Printf("%02d", month); break;
			case 'y': stream << Printf("%02d", year); break;
			default: stream << *pFormat; break;
			}
		}
		else
		{
			stream << *pFormat;
		}
		++pFormat;
	}
	return stream.str();
}

std::string DateTime::ToTimeString() const
{
	char buffer[sizeof("00-00-00")];
	sprintf_s(buffer, "%02d-%02d-%02d", GetHours(), GetMinutes(), GetSeconds());
	return std::string(buffer);
}

bool DateTime::operator<=(const DateTime& other) const
{
	return m_Ticks <= other.m_Ticks;
}

bool DateTime::operator<(const DateTime& other) const
{
	return m_Ticks < other.m_Ticks;
}

TimeSpan DateTime::operator+(const DateTime& other) const
{
	return TimeSpan(m_Ticks + other.m_Ticks);
}

TimeSpan DateTime::operator-(const DateTime& other) const
{
	return TimeSpan(m_Ticks - other.m_Ticks);
}

double DateTime::GetJulianDay() const
{
	return (double)(1721425.5 + m_Ticks / Time::TicksPerDay);
}
