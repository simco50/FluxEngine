#pragma once
class Subsystem;

class Context
{
public:
	Context() {}
	~Context() {}

	Subsystem* GetSubsystem(StringHash type) const
	{
		auto pIt = m_Systems.find(type);
		if (pIt == m_Systems.end())
		{
			return nullptr;
		}
		return pIt->second.get();
	}

	template<typename T>
	T* GetSubsystem(bool required = true) const
	{
		T* pSystem = static_cast<T*>(GetSubsystem(T::GetTypeStatic()));

		if (pSystem == nullptr)
		{
			if (required)
			{
				FLUX_LOG(ERROR, "[Context::GetSubsystem] System '%s' is not registered", T::GetTypeNameStatic().c_str());
			}
			return nullptr;
		}
		return pSystem;
	}

	template<typename T, typename ...Args>
	T* RegisterSubsystem(Args... args)
	{
		auto pIt = m_Systems.find(T::GetTypeStatic());
		if (pIt != m_Systems.end())
		{
			FLUX_LOG(ERROR, "[Content::RegisterSubsystem] > A subsystem with type '%s' is already registered", T::GetTypeNameStatic().c_str());
			return nullptr;
		}
		std::unique_ptr<T> pSystem = make_unique<T>(this, args...);
		StringHash type = pSystem->GetType();
		m_Systems[type] = std::move(pSystem);
		return (T*)m_Systems[type].get();
	}

private:
	//A collection of subsystems
	map<StringHash, unique_ptr<Subsystem>> m_Systems;
};