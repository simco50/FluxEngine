#pragma once
#include "Object.h"
class Subsystem;

class Context
{
public:
	Context();
	~Context();

	void InitSDLSystem(unsigned int flag);
	void ShutdownSDL();

	Subsystem* GetSubsystem(StringHash type, bool required = true) const;

	template<typename T>
	T* GetSubsystem(bool required = true) const
	{
		T* pSystem = static_cast<T*>(GetSubsystem(T::GetTypeStatic()));
		if (pSystem == nullptr)
		{
			if (required)
			{
				FLUX_LOG(Warning, "[Context::GetSubsystem] System '%s' is not registered", T::GetTypeNameStatic());
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
			FLUX_LOG(Warning, "[Content::RegisterSubsystem] > A subsystem with type '%s' is already registered", T::GetTypeNameStatic());
			return nullptr;
		}
		m_SystemCache.push_back(std::make_unique<T>(this, std::forward<Args>(args)...));
		Subsystem* pSystem = m_SystemCache[m_SystemCache.size() - 1].get();
		StringHash type = pSystem->GetType();
		m_Systems[type] = pSystem;
		return static_cast<T*>(pSystem);
	}


	template<typename T>
	void RegisterFactory()
	{
		m_Factories[T::GetTypeStatic()] = [](Context* pContext) { return static_cast<Object*>(new T(pContext)); };
	}

	Object* CreateObject(const char* pClassName, bool assertOnFailure = false)
	{
		StringHash hash = StringHash(pClassName);
		auto pIt = m_Factories.find(hash);
		if (pIt != m_Factories.end())
		{
			return pIt->second(this);
		}
		checkf(assertOnFailure, "[Context::CreateObject] Type is not registered");
		return nullptr;
	}

private:
	//A collection of subsystems
	std::unordered_map<StringHash, Subsystem*> m_Systems;
	//Vector to keep order of destruction
	std::vector<std::unique_ptr<Subsystem>> m_SystemCache;

	using ObjectFactoryFunction = Object * (*)(Context*);
	std::unordered_map<StringHash, ObjectFactoryFunction> m_Factories;

	int m_SdlInits = 0;
};