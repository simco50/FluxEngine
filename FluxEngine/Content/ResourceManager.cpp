//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "ResourceManager.h"
#include "MeshLoader.h"
#include "TextureLoader.h"
#include "ParticleSystemLoader.h"
#include "Rendering\Core\Graphics.h"

vector<Loader*> ResourceManager::m_Loaders = vector<Loader*>();
Graphics* ResourceManager::m_pGraphics = nullptr;
bool ResourceManager::m_IsInitialized = false;

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
	Initialize(m_pGraphics);
}

void ResourceManager::Initialize(Graphics* pGraphics)
{
	if(!m_IsInitialized)
	{
		m_pGraphics = pGraphics;
		m_IsInitialized = true;
		AddLoader(new MeshLoader());
		AddLoader(new TextureLoader());
		AddLoader(new ParticleSystemLoader());
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
	loader->SetDevice(m_pGraphics);
}
