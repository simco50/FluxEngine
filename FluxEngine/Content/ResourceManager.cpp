//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "ResourceManager.h"
#include "EffectLoader.h"
#include "MeshLoader.h"
#include "TextureLoader.h"
#include "SpriteFontLoader.h"

vector<Loader*> ResourceManager::m_Loaders = vector<Loader*>();
ID3D11Device* ResourceManager::m_pDevice = nullptr;
bool ResourceManager::m_IsInitialized = false;

ResourceManager::ResourceManager()
{
}


ResourceManager::~ResourceManager(void)
{
}

void ResourceManager::Release()
{
	for(Loader *pLoader : m_Loaders)
	{	
		pLoader->Unload();
		SafeDelete(pLoader);
	}

	m_Loaders.clear();
}

void ResourceManager::Reset()
{
	Release();
	m_IsInitialized = false;
	Initialize(m_pDevice);
}

void ResourceManager::Initialize(ID3D11Device* pDevice)
{
	if(!m_IsInitialized)
	{
		m_pDevice = pDevice;
		m_IsInitialized = true;
		AddLoader(new EffectLoader());
		AddLoader(new MeshLoader());
		AddLoader(new TextureLoader());
		AddLoader(new SpriteFontLoader());
	}
}

void ResourceManager::AddLoader(Loader* loader)
{ 
	for(Loader *ldr:m_Loaders)
	{	
		if(ldr->GetType()==loader->GetType())
		{
			SafeDelete(loader);
			break;
		}
	}

	m_Loaders.push_back(loader);
	loader->SetDevice(m_pDevice);
}
