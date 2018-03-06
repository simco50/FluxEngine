#pragma once
#include "Core\Subsystem.h"

class Resource;

class ResourceManager : public Subsystem
{
	FLUX_OBJECT(ResourceManager, Subsystem)

private:
	using ResourceGroup = std::map<std::string, Resource*>;
	using ResourceCache = std::map<StringHash, ResourceGroup>;

public:
	ResourceManager(Context* pContext);
	~ResourceManager();

	//Get the resource, if it is already loaded, take that
	template <typename T>
	T* Load(const std::string& filePath)
	{
		Resource* pResource = FindResource(filePath, T::GetTypeStatic());
		if (pResource)
			return (T*)pResource;
		pResource = new T(m_pContext);
		if (!LoadResourcePrivate(pResource, filePath))
		{
			delete pResource;
			return nullptr;
		}
		m_Resources[T::GetTypeStatic()][filePath] = pResource;
		return (T*)pResource;
	}

	//Reload the resource
	bool Reload(Resource* pResource);
	//Map resource to another file
	bool Reload(Resource* pResource, const std::string& filePath);

	void Unload(Resource*& pResource);

	std::vector<Resource*> GetResourcesOfType(StringHash type);
private:
	bool LoadResourcePrivate(Resource* pResource, const std::string& filePath);
	Resource* FindResource(const std::string& filePath, const StringHash type);

	ResourceCache m_Resources;
};