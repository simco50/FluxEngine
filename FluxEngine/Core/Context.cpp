#include "FluxEngine.h"
#include "Context.h"

#include <SDL.h>

Context::Context()
{

}

Context::~Context()
{
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

Subsystem* Context::GetSubsystem(StringHash type) const
{
	auto pIt = m_Systems.find(type);
	if (pIt == m_Systems.end())
	{
		return nullptr;
	}
	return pIt->second;
}

