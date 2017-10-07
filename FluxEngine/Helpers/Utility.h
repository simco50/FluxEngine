#pragma once

#define BIND_AND_CHECK_SEMANTIC(variable, semantic, as)\
{variable = m_pEffect->GetVariableBySemantic(semantic)->as();\
stringstream stream;\
stream << "Variable with semantic '" << semantic << "' not found";\
if(! variable->IsValid()) Console::LogError(stream.str());}

#define BIND_AND_CHECK_NAME(variable, name, as)\
variable = m_pEffect->GetVariableByName(#name)->as();\
if(! variable->IsValid()) Console::LogFormat(LogType::ERROR, "Variable with name '%s' not found", L#name);

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
inline void SafeRelease(T*& object)
{
	if (object)
	{
		((IUnknown*)object)->Release();
		object = nullptr;
	}
}