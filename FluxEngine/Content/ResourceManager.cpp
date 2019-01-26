#include "FluxEngine.h"
#include "ResourceManager.h"
#include "Resource.h"
#include "FileSystem\File\File.h"
#include "FileSystem\FileWatcher.h"
#include "Image.h"
#include "Database.h"

ResourceManager::ResourceManager(Context* pContext)
	: Subsystem(pContext)
{
	pContext->RegisterFactory<Image>();
	pContext->RegisterFactory<Database>();
}

ResourceManager::~ResourceManager()
{
	for (auto& resourceGroupEntry : m_Resources)
	{
		for (auto& resourceEntry : resourceGroupEntry.second)
		{
			delete resourceEntry.second;
		}
	}
}

void ResourceManager::Update()
{
	AUTOPROFILE(ResourceManager_Update);
	if (m_pResourceWatcher)
	{
		std::string resource;
		while (m_pResourceWatcher->GetNextChange(resource))
		{
			Paths::NormalizeInline(resource);
			Reload(resource);
			ReloadDependencies(resource);
		}
	}
}

Resource* ResourceManager::Load(const std::string& filePath, const char* typeName)
{
	Resource* pResource = NewObject<Resource>(StringHash(typeName));
	if (pResource)
	{
		LoadResourcePrivate(pResource, filePath);
		StringHash fileHash(filePath);
		StringHash typeHash(typeName);
		m_Resources[typeHash][fileHash] = pResource;
	}
	else
	{
		delete pResource;
	}
	return pResource;
}

void ResourceManager::EnableAutoReload(bool enable)
{
	if (enable && m_pResourceWatcher == nullptr)
	{
		m_pResourceWatcher = std::make_unique<FileWatcher>();
		m_pResourceWatcher->StartWatching(Paths::ResourcesDir(), true);
	}
	else
	{
		m_pResourceWatcher.reset();
	}
}

bool ResourceManager::Reload(Resource* pResource)
{
	if (pResource == nullptr)
	{
		return false;
	}
	return LoadResourcePrivate(pResource, pResource->GetFilePath());
}

bool ResourceManager::Reload(Resource* pResource, const std::string& filePath)
{
	if (pResource == nullptr)
	{
		return false;
	}
	ResourceGroup& resourceGroup = m_Resources[pResource->GetType()];
	auto pIt = resourceGroup.find(StringHash(pResource->GetFilePath()));
	if (pIt != resourceGroup.end())
	{
		resourceGroup.erase(pIt);
	}
	std::string path = Paths::Normalize(filePath);
	StringHash hash(path);
	resourceGroup[hash] = pResource;
	return LoadResourcePrivate(pResource, path);
}

bool ResourceManager::Reload(const std::string& filePath)
{
	StringHash path(Paths::Normalize(filePath));
	for (auto& groupPair : m_Resources)
	{
		for (auto& resourcePair : groupPair.second)
		{
			if (resourcePair.first == path)
			{
				if (Reload(resourcePair.second))
				{
					FLUX_LOG(Info, "[ResourceManager::Reload] Reloaded '%s'", Paths::GetFileName(filePath).c_str());
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
	{
		return;
	}

	ResourceGroup& resourceGroup = m_Resources[pResource->GetType()];
	auto pIt = resourceGroup.find(StringHash(pResource->GetFilePath()));
	if (pIt != resourceGroup.end())
	{
		resourceGroup.erase(pIt);
	}
	delete pResource;
	pResource = nullptr;
}

bool ResourceManager::ReloadDependencies(const std::string& resourcePath)
{
	bool success = true;
	StringHash hash(Paths::Normalize(resourcePath));
	auto pIt = m_ResourceDependencies.find(hash);
	if (pIt != m_ResourceDependencies.end())
	{
		FLUX_LOG(Info, "[ResourceManager::ReloadDependencies] %s has %d dependencies. Reloading dependencies", resourcePath.c_str(), pIt->second.size());
		std::vector<std::string> dependants = pIt->second;
		for (const std::string& path : dependants)
		{
			if (!Reload(path))
			{
				success = false;
			}
		}
	}
	return success;
}

void ResourceManager::AddResourceDependency(Resource* pResource, const std::string& filePath)
{
	const std::string& name = pResource->GetFilePath();
	StringHash hash(Paths::Normalize(filePath));
	m_ResourceDependencies[hash].push_back(name);
}

void ResourceManager::ResetDependencies(Resource* pResource)
{
	std::string path = Paths::Normalize(pResource->GetFilePath());
	for (auto pIt = m_ResourceDependencies.begin(); pIt != m_ResourceDependencies.end(); ++pIt)
	{
		auto it = std::remove_if(pIt->second.begin(), pIt->second.end(), [path](const std::string& name) {return name == path; });
		size_t distance = std::distance(pIt->second.begin(), it);
		if (distance < pIt->second.size())
		{
			pIt->second.resize(distance);
		}
	}
}

bool ResourceManager::LoadResourcePrivate(Resource* pResource, const std::string& filePath) const
{
	pResource->SetFilePath(filePath);
	std::unique_ptr<File> pFile = FileSystem::GetFile(filePath);
	if (pFile == nullptr)
	{
		FLUX_LOG(Warning, "[ResourceManager::LoadResourcePrivate] > Failed to load %s at '%s'. File not found", pResource->GetTypeName(), filePath.c_str());
		return false;
	}
	if (pFile->OpenRead() == false)
	{
		FLUX_LOG(Warning, "[ResourceManager::LoadResourcePrivate] > Failed to open file '%s'", filePath.c_str());
		return false;
	}
	pFile->SetSource(filePath);

	return LoadResourcePrivate(pResource, *pFile);
}

bool ResourceManager::LoadResourcePrivate(Resource* pResource, InputStream& inputStream) const
{
	if (!pResource->Load(inputStream))
	{
		FLUX_LOG(Warning, "[ResourceManager::LoadResourcePrivate] > Failed to load %s at '%s'", pResource->GetTypeName(), inputStream.GetSource().c_str());
		return false;
	}
	return true;
}

Resource* ResourceManager::FindResource(const std::string& filePath, const StringHash type)
{
	ResourceGroup& group = m_Resources[type];
	auto pIt = group.find(StringHash(filePath));
	if (pIt != group.end())
	{
		return pIt->second;
	}
	return nullptr;
}