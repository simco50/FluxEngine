#pragma once

#define STR(x) #x
#define STRINGIFY(x) STR(x)

#define FLUX_LOG_FMOD(fmod) \
Console::LogFmodResult(fmod)

#define FLUX_LOG_HR(description, hr) \
Console::LogHRESULT(description, hr)

#define FLUX_LOG(type, description, ...) \
Console::LogFormat(LogType::type, description, __VA_ARGS__);

#define FLUX_LOG_INFO(isVerbose, type, description, ...) \
if(isVerbose) \
{ \
	Console::LogFormat(LogType::Info, description, __VA_ARGS__); \
} \
else \
{ \
	Console::LogFormat(LogType::VeryVerbose, description, __VA_ARGS__); \
}

#ifdef _DEBUG

#define HR(command)\
{HRESULT r = command;\
Console::LogHRESULT(__FILE__ "\n" __FUNCTION__ "()\nLine:" STRINGIFY(__LINE__) "\nAction: " #command, r);}

#else

#define HR(command) command

#endif