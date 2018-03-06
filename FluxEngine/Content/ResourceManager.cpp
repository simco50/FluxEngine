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
	for (std::pair<std::string, Resource*> pResource : m_Resources)
		SafeDelete(pResource.second);
	m_Resources.clear();
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
	auto pIt = m_Resources.find(pResource->GetName());
	if(pIt != m_Resources.end())
		m_Resources.erase(pIt);
	m_Resources[filePath] = pResource;
	return LoadResourcePrivate(pResource, filePath);
}

std::vector<Resource*> ResourceManager::GetResourcesOfType(StringHash type)
{
	std::vector<Resource*> resources;
	for (const auto& p : m_Resources)
	{
		if (p.second->IsTypeOf(type))
			resources.push_back(p.second);
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
		FLUX_LOG(Error, "Failed to load %s at '%s'", pResource->GetTypeName().c_str(), filePath.c_str());
		return false;
	}
	return true;
}
