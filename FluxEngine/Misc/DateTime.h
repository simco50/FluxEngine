#pragma once
#include "TimeSpan.h"

struct DateTime
{
	DateTime();
	explicit DateTime(unsigned long long ticks);
	DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond);

	void Split(TimeStamp& timestamp) const;
	void GetDate(int& year, int& month, int& day) const;

	static int UtcNow();
	static DateTime Now();

	std::string ToString(const char* pFormat) const;
	std::string ToTimeString() const;
	std::string ToDateString() const;

	int GetMilliSeconds() const;
	int GetSeconds() const;
	int GetMinutes() const;
	int GetHours() const;
	int GetHours12() const;

	int GetDay() const;
	int GetMonth() const;
	int GetYear() const;
	bool IsMorning() const;

	std::string ToString() const;

	bool operator==(const DateTime& other) const;
	bool operator!=(const DateTime& other) const;
	bool operator<(const DateTime& other) const;
	bool operator>(const DateTime& other) const;
	bool operator<=(const DateTime& other) const;
	bool operator>=(const DateTime& other) const;

	TimeSpan operator+(const DateTime& other) const;
	TimeSpan operator-(const DateTime& other) const;

	unsigned long long m_Ticks;

private:

	double GetJulianDay() const;
};