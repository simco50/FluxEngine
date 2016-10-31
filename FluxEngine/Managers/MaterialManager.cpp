#include "stdafx.h"
#include "MaterialManager.h"
#include "../Materials/Material.h"

MaterialManager::MaterialManager()
{
}


MaterialManager::~MaterialManager()
{
	for (size_t i = 0; i < m_Materials.size(); i++)
		SafeDelete(m_Materials[i]);
}

void MaterialManager::Initialize(GameContext* pGameContext)
{
	m_pGameContext = pGameContext;
}

int MaterialManager::AddMaterial(Material* pMaterial)
{
	pMaterial->Initialize(m_pGameContext);
	m_Materials.push_back(pMaterial);
	return m_Materials.size() - 1;
}

template <typename T>
T* MaterialManager::GetMaterial(int id)
{
	if (id >= m_Materials.size())
	{
		DebugLog::LogFormat(LogType::ERROR, L"Material with ID %i does not exist!", id);
		return;
	}
	return static_cast<T*>(m_Materials[id]);
}
