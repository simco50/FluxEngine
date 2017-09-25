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

void GameManager::Initialize(EngineContext* pEngineContext)
{	
	UNREFERENCED_PARAMETER(pEngineContext);
}

void GameManager::Update()
{
}

LRESULT GameManager::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(message);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	return 0;
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