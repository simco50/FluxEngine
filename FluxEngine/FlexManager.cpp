#include "stdafx.h"
#include "FlexManager.h"
#include "Physics/Flex/FlexSystem.h"

FlexManager::FlexManager()
{
}


FlexManager::~FlexManager()
{
}

void FlexManager::Initialize()
{
	m_pFlexLibrary = NvFlexInit(NV_FLEX_VERSION);
}

void FlexManager::Shutdown()
{
	NvFlexShutdown(m_pFlexLibrary);
}

void FlexManager::Update()
{
	for(FlexSystem* pSystem : m_pSystems)
	{
		pSystem->UpdateSolver();
	}
}

FlexSystem* FlexManager::CreateSystem()
{
	FlexSystem* pSystem = new FlexSystem(m_pFlexLibrary);
	m_pSystems.push_back(pSystem);
}
