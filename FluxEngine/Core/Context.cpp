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
	auto pIt = m_Factories.find(typeHash);
	if (pIt != m_Factories.end())
	{
		return pIt->second(this);
	}
	checkf(assertOnFailure, "[Context::CreateObject] Type is not registered");
	return nullptr;
}

std::vector<const TypeInfo*> Context::GetAllTypesOf(StringHash type, bool includeAbstract)
{
	std::vector<const TypeInfo*> typeData;
	GetAllTypesOf(type, typeData, includeAbstract);
	return typeData;
}

void Context::GetAllTypesOf(StringHash type, std::vector<const TypeInfo*>& typeData, bool includeAbstract)
{
	auto pIt = m_TypeTrees.find(type);
	if (pIt != m_TypeTrees.end())
	{
		for (const TypeInfo* pTypeInfo : pIt->second)
		{
			if (includeAbstract || !pTypeInfo->IsAbstract())
			{
				typeData.push_back(pTypeInfo);
			}
			GetAllTypesOf(pTypeInfo->GetType(), typeData, includeAbstract);
		}
	}
}
