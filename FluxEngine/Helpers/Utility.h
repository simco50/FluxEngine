#pragma once
//Macros

#ifdef _DEBUG
#define HR(command)\
{HRESULT r = command;\
stringstream stream;\
stream << typeid(*this).name() << "::" << __func__ << "()" << endl;\
stream << "Line: " << __LINE__  << endl;\
stream << "Action: " << #command ;\
Console::LogHRESULT(stream.str(), r);}
#else
#define HR(command) command;
#endif

#define BIND_AND_CHECK_SEMANTIC(variable, semantic, as)\
{variable = m_pEffect->GetVariableBySemantic(semantic)->as();\
stringstream stream;\
stream << "Variable with semantic '" << semantic << "' not found";\
if(! variable->IsValid()) Console::Log(stream.str(), LogType::ERROR);}

#define BIND_AND_CHECK_NAME(variable, name, as)\
variable = m_pEffect->GetVariableByName(#name)->as();\
if(! variable->IsValid()) Console::LogFormat(LogType::ERROR, "Variable with name '%s' not found", L#name);

#define ERROR_TEXTURE "./Resources/Textures/ErrorTexture.jpg"

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
		object->Release();
		object = nullptr;
	}
}