#include "FluxEngine.h"
#include "Context.h"

#include <SDL.h>

Context::Context()
{

}

Context::~Context()
{
	for (Subsystem* pSystem : m_SystemCache)
	{
		delete pSystem;
	}
	m_SystemCache.clear();
	if (m_SdlInits > 0)
	{
		FLUX_LOG(Warning, "SDL hasn't been properly shut down");
	}
}

void Context::InitSDLSystem(unsigned int flag)
{
	if (m_SdlInits == 0)
	{
		SDL_Init(0);
	}

	unsigned int flagsToEnable = flag & ~SDL_WasInit(0);
	SDL_InitSubSystem(flagsToEnable);
	++m_SdlInits;
}

void Context::ShutdownSDL()
{
	--m_SdlInits;
	if (m_SdlInits == 0)
	{
		SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
		SDL_Quit();
	}
}

Subsystem* Context::GetSubsystem(StringHash type, bool required) const
{
	auto pIt = m_Systems.find(type);
	if (pIt == m_Systems.end())
	{
		if (required)
		{
			checkNoEntry();
		}
		return nullptr;
	}
	return pIt->second;
}

Object* Context::NewObject(const StringHash typeHash, bool assertOnFailure /*= false*/)
{
	assertOnFailure;
	auto pIt = m_RegisteredTypes.find(typeHash);
	if (pIt != m_RegisteredTypes.end())
	{
		if (pIt->second->IsAbstract())
		{
			checkf(assertOnFailure, "[Context::CreateObject] Can't create instance of abstract class");
			return nullptr;
		}
		return pIt->second->CreateInstance(this);
	}
	checkf(assertOnFailure, "[Context::CreateObject] Type is not registered");
	return nullptr;
}

std::vector<const TypeInfo*> Context::GetAllTypesOf(StringHash type, bool includeAbstract)
{
	std::vector<const TypeInfo*> typeData;
	for (const auto& pair : m_RegisteredTypes)
	{
		if (includeAbstract || !pair.second->IsAbstract())
		{
			if (pair.second->IsTypeOf(type))
			{
				typeData.push_back(pair.second);
			}
		}
	}
	return typeData;
}
