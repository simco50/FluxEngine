#pragma once
class Subsystem;

class Context
{
public:
	Context();
	~Context();

	template<typename T>
	T* GetSubsystem()
	{
		auto pIt = m_Systems.find(T::GetTypeStatic());
		if (pIt == m_Systems.end())
			return nullptr;
		return (T*)pIt->second.get();
	}

	template<typename T>
	T* RegisterSubsystem(unique_ptr<T> pSystem)
	{
		StringHash type = pSystem->GetType();
		m_Systems[type] = std::move(pSystem);
		return (T*)m_Systems[type].get();
	}

private:
	map<StringHash, unique_ptr<Subsystem>> m_Systems;
};