#pragma once

#ifdef _DEBUG
//Assert if the expression is false
#define check(x) \
if(!(x)) { Console::LogFormat(LogType::Warning, "Assertion at (%i) %s", __LINE__, __FILE__); DebugBreak();} \
//Assert with description if the expression is false
#define checkf(x, description) \
if(!(x)) { Console::LogFormat(LogType::Warning, "Assertion at (%i) %s: %s", __LINE__, __FILE__, description); DebugBreak();} \
//Asserts when this called
#define checkNoEntry() \
Console::LogFormat(LogType::Warning, "(%i) %s: Block should never be called", __LINE__, __FILE__); DebugBreak()
#else
//Ingored
#define check(x) 0
//Ingored
#define checkf(x, description) 0
//Ingored
#define checkNoEntry() 0
#endif