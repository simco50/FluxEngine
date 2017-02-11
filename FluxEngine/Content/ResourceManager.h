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

	ResourceManager();
	~ResourceManager(void);

	template<class T> 
	static T* Load(const wstring& assetFile)
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
		wstringstream stream;
		stream << L"No appropriate ResourceLoader found for: " << assetFile;
		DebugLog::Log(stream.str(), LogType::ERROR);
		return nullptr;
	}

	template<class T>
	static T* Reload(const wstring& assetFile)
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
		wstringstream stream;
		stream << L"No appropriate ResourceLoader found for: " << assetFile;
		DebugLog::Log(stream.str(), LogType::ERROR);
		return nullptr;
	}

	template<class T>
	static T* Load_Unmanaged(const wstring& assetFile)
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
		wstringstream stream;
		stream << L"No appropriate ResourceLoader found for: " << assetFile;
		DebugLog::Log(stream.str(), LogType::ERROR);
		return nullptr;
	}

	static void Release();
	static void Reset();

private:
	static vector<Loader*> m_Loaders;
	static ID3D11Device* m_pDevice;
	static bool m_IsInitialized;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	ResourceManager(const ResourceManager& t);
	ResourceManager& operator=(const ResourceManager& t);
};

