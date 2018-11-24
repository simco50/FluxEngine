#pragma once

#define ERROR_TEXTURE "./Resources/Textures/ErrorTexture.jpg"

//Deletes the copy constructor and assignment operator
#define DELETE_COPY(className) \
className(const className& other) = delete; \
className& operator=(const className& other) = delete;

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

#ifdef x86
constexpr StringHash val_32_const = 0x811c9dc5;
constexpr StringHash prime_32_const = 0x1000193;
inline constexpr uint32_t HashString(const char* const str, const StringHash value = val_32_const) noexcept
{
	return (str[0] == '\0') ? value : HashString(&str[1], (value ^ StringHash(str[0])) * prime_32_const);
}
#else
constexpr StringHash val_64_const = 0xcbf29ce484222325;
constexpr StringHash prime_64_const = 0x100000001b3;
inline constexpr StringHash HashString(const char* const str, const StringHash value = val_64_const) noexcept
{
	return (str[0] == '\0') ? value : HashString(&str[1], (value ^ StringHash(str[0])) * prime_64_const);
}
#endif

inline StringHash HashString(const std::string& string)
{
	return HashString(string.c_str(), string.length());
}