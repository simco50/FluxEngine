#pragma once
class Object;

class Context
{
public:
	Context();
	~Context();

	template<typename T>
	void RegisterSubsystem(T* pSystem)
	{
		string typeName = string(typeid(pSystem).name());
		if (m_Subsystems.find(typeName) == m_Subsystems.end())
		{
			m_Subsystems[typeName] = pSystem;
		}
	}

	template<typename T>
	T* GetSubsystem()
	{
		string typeName = string(typeid(T*).name());
		auto pPtr = m_Subsystems.find(typeName);
		if (pPtr != m_Subsystems.end())
			return (T*)pPtr->second;
		return nullptr;
	}

private:
	map<string, void*> m_Subsystems;
};

