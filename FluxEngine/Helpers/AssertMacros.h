#pragma once

#ifdef _DEBUG
//Assert if the expression is false
#define check(x) \
if((x)) { } \
else { Console::LogFormat(LogType::Warning, "Assertion at (%i) %s. '%s'", __LINE__, __FILE__, #x); __debugbreak(); } \

//Assert with description if the expression is false
#define checkf(x, description) \
if((x)) { } \
else { Console::LogFormat(LogType::Warning, "Assertion at (%i) %s. '%s' : %s", __LINE__, __FILE__, #x, description); __debugbreak();} \

//Asserts when this called
#define checkNoEntry() \
Console::LogFormat(LogType::Warning, "(%i) %s: Block should never be called", __LINE__, __FILE__); __debugbreak()

#else
//Ingored
#define check(x) 0
//Ingored
#define checkf(x, description) 0
//Ingored
#define checkNoEntry() 0
#endif