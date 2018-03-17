#include "FluxEngine.h"
#include "ResourceManager.h"
#include "Resource.h"
#include "FileSystem\File\File.h"

ResourceManager::ResourceManager(Context* pContext) :
	Subsystem(pContext)
{

}

ResourceManager::~ResourceManager()
{
	for (auto& resourceGroupEntry : m_Resources)
	{
		for (auto& resourceEntry : resourceGroupEntry.second)
		{
			SafeDelete(resourceEntry.second);
		}
	}
}

bool ResourceManager::Reload(Resource* pResource)
{
	if (pResource == nullptr)
		return false;
	return LoadResourcePrivate(pResource, pResource->GetName());
}

bool ResourceManager::Reload(Resource* pResource, const std::string& filePath)
{
	if (pResource == nullptr)
		return false;
	ResourceGroup& resourceGroup = m_Resources[pResource->GetType()];
	auto pIt = resourceGroup.find(pResource->GetName());
	if(pIt != resourceGroup.end())
		resourceGroup.erase(pIt);
	resourceGroup[filePath] = pResource;
	return LoadResourcePrivate(pResource, filePath);
}

bool ResourceManager::Reload(const std::string& filePath)
{
	for (auto& groupPair : m_Resources)
	{
		for (auto& resourcePair : groupPair.second)
		{
			if (resourcePair.first == filePath)
			{
				if (Reload(resourcePair.second))
				{
					FLUX_LOG(Info, "[ResourceManager::Reload] > Reloaded %s", filePath.c_str());
					return true;
				}
				return false;
			}
		}
	}
	return false;
}

void ResourceManager::Unload(Resource*& pResource)
{
	if (pResource == nullptr)
		return;

	ResourceGroup& resourceGroup = m_Resources[pResource->GetType()];
	auto pIt = resourceGroup.find(pResource->GetName());
	if (pIt != resourceGroup.end())
		resourceGroup.erase(pIt);
	delete pResource;
	pResource = nullptr;
}

std::vector<Resource*> ResourceManager::GetResourcesOfType(StringHash type)
{
	std::vector<Resource*> resources;
	for (auto& resourceGroup : m_Resources)
	{
		if (resourceGroup.second.size() > 0 && resourceGroup.second.begin()->second->IsTypeOf(type))
		{
			for (auto& p : resourceGroup.second)
			{
				if (p.second->IsTypeOf(type))
					resources.push_back(p.second);
			}
		}
	}
	return resources;
}

bool ResourceManager::LoadResourcePrivate(Resource* pResource, const std::string& filePath)
{
	pResource->SetName(filePath);
	std::unique_ptr<File> pFile = FileSystem::GetFile(filePath);
	if (pFile == nullptr)
		return false;
	if (pFile->Open(FileMode::Read, ContentType::Binary) == false)
		return false;
	pFile->SetSource(filePath);

	if (!pResource->Load(*pFile.get()))
	{
		FLUX_LOG(Warning, "[ResourceManager::LoadResourcePrivate] > Failed to load %s at '%s'", pResource->GetTypeName().c_str(), filePath.c_str());
		return false;
	}
	return true;
}

Resource* ResourceManager::FindResource(const std::string& filePath, const StringHash type)
{
	ResourceGroup& group = m_Resources[type];
	auto pIt = group.find(filePath);
	if (pIt != group.end())
		return pIt->second;
	return nullptr;
}