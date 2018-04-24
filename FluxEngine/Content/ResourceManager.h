#pragma once
#include "Core\Subsystem.h"

class Resource;
class FileWatcher;

class ResourceManager : public Subsystem
{
	FLUX_OBJECT(ResourceManager, Subsystem)

private:
	using ResourceGroup = std::map<std::string, Resource*>;
	using ResourceCache = std::map<StringHash, ResourceGroup>;

public:
	ResourceManager(Context* pContext);
	~ResourceManager();

	void Update();

	//Get the resource, if it is already loaded, take that
	template <typename T>
	T* Load(const std::string& filePath)
	{
		std::string path = Paths::Normalize(filePath);
		Resource* pResource = FindResource(path, T::GetTypeStatic());
		if (pResource)
			return (T*)pResource;
		pResource = new T(m_pContext);
		if (!LoadResourcePrivate(pResource, path))
		{
			delete pResource;
			return nullptr;
		}
		m_Resources[T::GetTypeStatic()][path] = pResource;
		return (T*)pResource;
	}

	void EnableAutoReload();

	//Reload the resource
	bool Reload(Resource* pResource);
	//Map resource to another file
	bool Reload(Resource* pResource, const std::string& filePath);
	bool Reload(const std::string& filePath);
	void Unload(Resource*& pResource);

	bool ReloadDependencies(const std::string& resourcePath);

	void AddResourceDependency(Resource* pResource, const std::string& filePath);
	void ResetDependencies(Resource* pResource);

	std::vector<Resource*> GetResourcesOfType(StringHash type);
private:
	bool LoadResourcePrivate(Resource* pResource, const std::string& filePath);
	Resource* FindResource(const std::string& filePath, const StringHash type);

	std::map<std::string, std::vector<std::string>> m_ResourceDependencies;

	std::unique_ptr<FileWatcher> m_pResourceWatcher;
	ResourceCache m_Resources;
};