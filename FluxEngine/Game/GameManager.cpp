#include "stdafx.h"
#include "GameManager.h"
#include "Scenegraph/SceneBase.h"
#include "Scenes/DemoScene.h"

GameManager::GameManager()
{
}

GameManager::~GameManager()
{
}

void GameManager::PrepareGame()
{

}

void GameManager::Initialize()
{	
}

void GameManager::Update()
{
}

void GameManager::AddScene(SceneBase* pScene)
{
	UNREFERENCED_PARAMETER(pScene);
}

void GameManager::ReloadScene(SceneBase* pScene)
{
	UNREFERENCED_PARAMETER(pScene);
}

void GameManager::LoadScene(const int index)
{
	UNREFERENCED_PARAMETER(index);
}