#pragma once

#define ERROR_TEXTURE "./Resources/Textures/ErrorTexture.jpg"

//Deletes the copy constructor and assignment operator
#define DELETE_COPY(className) \
className(const className& other) = delete; \
className& operator=(const className& other) = delete;

//Utility methods

inline StringHash HashString(const std::string& input)
{
	return std::hash<std::string>{}(input);
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

template<typename ...Args>
std::string Printf(const std::string& format, Args... args)
{
	auto size = std::snprintf(nullptr, 0, format.c_str(), args...);
	std::string output(size + 1, '\0');
	sprintf_s(&output[0], output.size(), format.c_str(), args...);
	return std::string(output.begin(), output.end() - 1);
}