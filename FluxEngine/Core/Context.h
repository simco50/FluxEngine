#pragma once
class Subsystem;

class Context
{
public:
	Context() {}
	~Context()
	{
		m_SystemCache.clear();
		if (m_SdlInits > 0)
			FLUX_LOG(Warning, "SDL hasn't been properly shut down");
	}

	Subsystem* GetSubsystem(StringHash type) const
	{
		auto pIt = m_Systems.find(type);
		if (pIt == m_Systems.end())
		{
			return nullptr;
		}
		return pIt->second;
	}

	void InitSDLSystem(unsigned int flag)
	{
		if (m_SdlInits == 0)
			SDL_Init(0);

		unsigned int flagsToEnable = flag & ~SDL_WasInit(0);
		SDL_InitSubSystem(flagsToEnable);
		++m_SdlInits;
	}

	void ShutdownSDL()
	{
		--m_SdlInits;
		if (m_SdlInits == 0)
		{
			SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
			SDL_Quit();
		}
	}

	template<typename T>
	T* GetSubsystem(bool required = true) const
	{
		T* pSystem = static_cast<T*>(GetSubsystem(T::GetTypeStatic()));
		if (pSystem == nullptr)
		{
			if (required)
			{
				FLUX_LOG(Error, "[Context::GetSubsystem] System '%s' is not registered", T::GetTypeNameStatic().c_str());
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
			FLUX_LOG(Error, "[Content::RegisterSubsystem] > A subsystem with type '%s' is already registered", T::GetTypeNameStatic().c_str());
			return nullptr;
		}
		m_SystemCache.push_back(std::make_unique<T>(this, args...));
		Subsystem* pSystem = m_SystemCache[m_SystemCache.size() - 1].get();
		StringHash type = pSystem->GetType();
		m_Systems[type] = pSystem;
		return static_cast<T*>(pSystem);
	}

private:
	//A collection of subsystems
	std::unordered_map<StringHash, Subsystem*> m_Systems;
	//Vector to keep order of destruction
	std::vector<std::unique_ptr<Subsystem>> m_SystemCache;

	int m_SdlInits = 0;
};