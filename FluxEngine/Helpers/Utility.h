#pragma once

#define ERROR_TEXTURE "./Resources/Textures/ErrorTexture.jpg"

//Deletes the copy constructor and assignment operator
#define DELETE_COPY(className) \
className(const className& other) = delete; \
className& operator=(const className& other) = delete;

//Utility methods
template<typename T>
inline void SafeDelete(T*& object)
{
	if (object)
	{
		delete object;
		object = nullptr;
	}
}

template<typename T>
inline void SafeDeleteArray(T*& object)
{
	if (object)
	{
		delete[] object;
		object = nullptr;
	}
}

template<typename T>
inline void SafeRelease(T*& object)
{
	if (object)
	{
		((IUnknown*)object)->Release();
		object = nullptr;
	}
}

inline string GetTimeStamp()
{
	time_t timer;
	time(&timer);
	tm localTime;
	localtime_s(&localTime, &timer);
	stringstream str;
	str << 1900 + localTime.tm_year << "-"
		<< setfill('0') << setw(2) << localTime.tm_mon + 1
		<< "-" << setw(2) << localTime.tm_mday
		<< "_" << setw(2) << localTime.tm_hour
		<< "-" << setw(2) << localTime.tm_min
		<< "-" << setw(2) << localTime.tm_sec;
	return str.str();
}
