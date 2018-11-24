#pragma once
#include "Core\Subsystem.h"

class Resource;
class FileWatcher;

class ResourceManager : public Subsystem
{
	FLUX_OBJECT(ResourceManager, Subsystem)

private:
	using ResourceGroup = std::map<StringHash, Resource*>;
	using ResourceCache = std::map<StringHash, ResourceGroup>;

public:
	ResourceManager(Context* pContext);
	~ResourceManager();

	DELETE_COPY(ResourceManager)

	void Update();

	//Get the resource, if it is already loaded, take that
	template <typename T>
	T* Load(const std::string& filePath)
	{
		std::string path = Paths::Normalize(filePath);
		Resource* pResource = FindResource(path, T::GetTypeStatic());
		if (pResource)
		{
			return static_cast<T*>(pResource);
		}
		pResource = new T(m_pContext);
		if (!LoadResourcePrivate(pResource, path))
		{
			delete pResource;
			return nullptr;
		}
		StringHash hash = HashString(path);
		m_Resources[T::GetTypeStatic()][hash] = pResource;
		return static_cast<T*>(pResource);
	}

	template <typename T>
	T* Load(InputStream& inputStream)
	{
		std::string path = Paths::Normalize(inputStream.GetSource());
		Resource* pResource = FindResource(path, T::GetTypeStatic());
		if (pResource)
		{
			return static_cast<T*>(pResource);
		}
		pResource = new T(m_pContext);
		if (!LoadResourcePrivate(pResource, inputStream))
		{
			delete pResource;
			return nullptr;
		}
		StringHash hash = HashString(path);
		m_Resources[T::GetTypeStatic()][hash] = pResource;
		return static_cast<T*>(pResource);
	}

	template<typename T>
	T* CreateResource(const std::string filePath)
	{
		std::string path = Paths::Normalize(filePath);
		Resource* pResource = new T(m_pContext);
		StringHash hash = HashString(path);
		m_Resources[T::GetTypeStatic()][hash] = pResource;
		return static_cast<T*>(pResource);
	}

	void EnableAutoReload(bool enable);

	//Reload the resource
	bool Reload(Resource* pResource);
	//Map resource to another file
	bool Reload(Resource* pResource, const std::string& filePath);
	bool Reload(const std::string& filePath);
	void Unload(Resource*& pResource);

	bool ReloadDependencies(const std::string& resourcePath);

	void AddResourceDependency(Resource* pResource, const std::string& filePath);

	//Remove all the reload dependencies other resources have to this resource
	void ResetDependencies(Resource* pResource);

private:
	bool LoadResourcePrivate(Resource* pResource, const std::string& filePath) const;
	bool LoadResourcePrivate(Resource* pResource, InputStream& inputStream) const;
	Resource* FindResource(const std::string& filePath, const StringHash type);

	std::map<StringHash, std::vector<std::string>> m_ResourceDependencies;

	std::unique_ptr<FileWatcher> m_pResourceWatcher;
	ResourceCache m_Resources;
};