#pragma once
#include "Resource.h"

class Resource;

class ResourceManager : public Singleton<ResourceManager>
{

public:
	ResourceManager() {}
	~ResourceManager();;

	template <typename T, typename ...Args>
	T* Load(const std::string& filePath, Args... args)
	{
		auto pIt = m_Resources.find(filePath);
		if (pIt != m_Resources.end())
			return (T*)(pIt->second);

		Resource* pResource = new T(args...);
		if (!LoadResourcePrivate(pResource, filePath))
		{
			delete pResource;
			return nullptr;
		}
		m_Resources[filePath] = pResource;
		return (T*)pResource;
	}

private:
	bool LoadResourcePrivate(Resource* pResource, const std::string& filePath);

	std::map<std::string, Resource*> m_Resources;
};