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

bool ResourceManager::LoadResourcePrivate(Resource* pResource, const std::string& filePath)
{
	std::unique_ptr<File> pFile = FileSystem::GetFile(filePath);
	if (pFile == nullptr)
		return false;
	if (pFile->Open(FileMode::Read, ContentType::Binary) == false)
		return false;
	pFile->SetSource(filePath);

	if (!pResource->Load(*pFile.get()))
	{
		FLUX_LOG(Error, "File with name '%s' not found.", filePath.c_str());
		return false;
	}
	return true;
}
