#include "stdafx.h"
#include "GameManager.h"
#include "Scenegraph/SceneBase.h"
#include "Scenes/DemoScene.h"

GameManager::GameManager()
{
}

GameManager::~GameManager()
{
	for (SceneBase*& pScene : m_pScenes)
	{
		SafeDelete(pScene);
	}
}

void GameManager::AddScene(SceneBase* pScene)
{
	m_pScenes.push_back(pScene);
}

void GameManager::ReloadScene(SceneBase* pScene)
{
	UNREFERENCED_PARAMETER(pScene);
}

void GameManager::LoadScene(const int index)
{
	m_ActiveScene = index;
	if (!m_pScenes[index]->IsInitialized())
		m_pScenes[index]->Initialize();
}