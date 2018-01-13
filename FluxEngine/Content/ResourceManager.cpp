#include "stdafx.h"
#include "ResourceManager.h"
#include "Resource.h"

ResourceManager::~ResourceManager()
{
	for (std::pair<std::string, Resource*> pResource : m_Resources)
		SafeDelete(pResource.second);
	m_Resources.clear();
}

bool ResourceManager::LoadResourcePrivate(Resource* pResource, const std::string& filePath)
{
	if (!pResource->Load(filePath))
	{
		FLUX_LOG(ERROR, "File with name '%s' not found.", filePath.c_str());
		return false;
	}
	return true;
}
