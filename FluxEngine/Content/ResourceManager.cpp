#include "FluxEngine.h"
#include "ResourceManager.h"
#include "Resource.h"
#include "FileSystem\File\File.h"
#include "FileSystem\FileWatcher.h"

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

void ResourceManager::Update()
{
	if (m_pResourceWatcher)
	{
		std::string resource;
		while (m_pResourceWatcher->GetNextChange(resource))
		{
			std::string filePath = Paths::Normalize("Resources/" + resource);
			Reload(filePath);
			ReloadDependencies(filePath);
		}
	}
}

void ResourceManager::EnableAutoReload()
{
	m_pResourceWatcher = std::make_unique<FileWatcher>();
	m_pResourceWatcher->StartWatching("Resources", true);
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
	std::string path = Paths::Normalize(filePath);
	resourceGroup[path] = pResource;
	return LoadResourcePrivate(pResource, path);
}

bool ResourceManager::Reload(const std::string& filePath)
{
	std::string path = Paths::Normalize(filePath);
	for (auto& groupPair : m_Resources)
	{
		for (auto& resourcePair : groupPair.second)
		{
			if (resourcePair.first == path)
			{
				if (Reload(resourcePair.second))
				{
					FLUX_LOG(Info, "[ResourceManager::Reload] > Reloaded %s", resourcePair.first.c_str());
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

bool ResourceManager::ReloadDependencies(const std::string& resourcePath)
{
	bool success = true;
	auto pIt = m_ResourceDependencies.find(Paths::Normalize(resourcePath));
	if (pIt != m_ResourceDependencies.end())
	{
		std::vector<std::string> dependants = pIt->second;
		for (const std::string& path : dependants)
		{
			if (!Reload(path))
				success = false;
		}
	}
	return success;
}

void ResourceManager::AddResourceDependency(Resource* pResource, const std::string& filePath)
{
	std::string name = pResource->GetName();
	m_ResourceDependencies[Paths::Normalize(filePath)].push_back(name);
}

void ResourceManager::ResetDependencies(Resource* pResource)
{
	std::string path = Paths::Normalize(pResource->GetName());
	for (auto pIt = m_ResourceDependencies.begin(); pIt != m_ResourceDependencies.end(); ++pIt)
	{
		auto it = std::remove_if(pIt->second.begin(), pIt->second.end(), [path](const std::string& name) {return name == path; });
		size_t distance = std::distance(pIt->second.begin(), it);
		if (distance < pIt->second.size())
			pIt->second.resize(distance);
	}
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
	{
		FLUX_LOG(Warning, "[ResourceManager::LoadResourcePrivate] > Failed to load %s at '%s'. File not found", pResource->GetTypeName().c_str(), filePath.c_str());
		return false;
	}
	if (pFile->Open(FileMode::Read) == false)
	{
		FLUX_LOG(Warning, "[ResourceManager::LoadResourcePrivate] > Failed to open file '%s'", filePath.c_str());
		return false;
	}
	pFile->SetSource(filePath);

	if (!pResource->Load(*pFile.get()))
	{
		FLUX_LOG(Warning, "[ResourceManager::LoadResourcePrivate] > Failed to load %s at '%s'", pResource->GetTypeName().c_str(), filePath.c_str());
		return false;
	}
	pResource->OnLoaded().Broadcast();
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