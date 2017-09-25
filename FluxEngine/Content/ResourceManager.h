#pragma once
#include <vector>
#include <typeinfo>
#include "ResourceLoader.h"

using namespace std;

class ResourceManager
{
public:
	static void Initialize(ID3D11Device* pDevice);
	static void AddLoader(Loader* loader);

	ResourceManager() {}
	~ResourceManager(){}

	ResourceManager(const ResourceManager& t) = delete;
	ResourceManager& operator=(const ResourceManager& t) = delete;

	template<class T> 
	static T* Load(const string& assetFile)
	{
		const type_info& ti = typeid(T);
		for(Loader* loader:m_Loaders)
		{
			const type_info& loadertype = loader->GetType();
			if(loadertype == ti)
			{
				T* pObj = (static_cast<ResourceLoader<T>*>(loader))->GetContent(assetFile);
				return pObj;
			}
		}
		FLUX_LOG(ERROR, "No appropriate ResourceLoader found for: %s", assetFile.c_str());
		return nullptr;
	}

	template<class T>
	static T* Reload(const string& assetFile)
	{
		const type_info& ti = typeid(T);
		for (Loader* loader : m_Loaders)
		{
			const type_info& loadertype = loader->GetType();
			if (loadertype == ti)
			{
				T* pObj = (static_cast<ResourceLoader<T>*>(loader))->GetContent_Reload(assetFile);
				return pObj;
			}
		}
		FLUX_LOG(ERROR, "No appropriate ResourceLoader found for: %s", assetFile.c_str());
		return nullptr;
	}

	template<class T>
	static T* Load_Unmanaged(const string& assetFile)
	{
		const type_info& ti = typeid(T);
		for (Loader* loader : m_Loaders)
		{
			const type_info& loadertype = loader->GetType();
			if (loadertype == ti)
			{
				T* pObj = (static_cast<ResourceLoader<T>*>(loader))->GetContent_Unmanaged(assetFile);
				return pObj;
			}
		}
		stringstream stream;
		stream << "No appropriate ResourceLoader found for: " << assetFile;
		FLUX_LOG(ERROR, stream.str());
		return nullptr;
	}

	static void Release();
	static void Reset();

private:
	static vector<Loader*> m_Loaders;
	static ID3D11Device* m_pDevice;
	static bool m_IsInitialized;
};

