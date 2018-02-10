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

template<typename T>
inline void PhysXSafeRelease(T*& object)
{
	if (object)
	{
		object->release();
		object = nullptr;
	}
}

inline std::string GetTimeStamp()
{
	time_t timer;
	time(&timer);
	tm localTime;
	localtime_s(&localTime, &timer);
	std::stringstream str;
	str << 1900 + localTime.tm_year << "-"
		<< std::setfill('0') << std::setw(2) << localTime.tm_mon + 1
		<< "-" << std::setw(2) << localTime.tm_mday
		<< "_" << std::setw(2) << localTime.tm_hour
		<< "-" << std::setw(2) << localTime.tm_min
		<< "-" << std::setw(2) << localTime.tm_sec;
	return str.str();
}

template<typename ...Args>
std::string Printf(const std::string& format, Args... args)
{
	auto size = std::snprintf(nullptr, 0, format.c_str(), args...);
	std::string output(size + 1, '\0');
	sprintf_s(&output[0], output.size(), format.c_str(), args...);
	return output;
}