#include "stdafx.h"
#include "GameManager.h"
#include "../Scenegraph/SceneBase.h"
#include "Scenes/SoftBodyScene.h"
#include "Scenes/ParticleScene.h"
#include "../Physics/Flex/FlexHelper.h"
#include "Scenes/StressTestScene.h"
#include "Scenes/DaeLogoScene.h"
#include "Scenes/ParameterDemoScene.h"
#include "Scenes/CollisionDemoScene.h"

GameManager::GameManager()
{
}

GameManager::~GameManager()
{
	for (size_t i = 0; i < m_pScenes.size(); i++)
	{
		SafeDelete(m_pScenes[i]);
	}
	flexShutdown();
}

void GameManager::PrepareGame()
{
	m_EngineContext.GameSettings.WindowStyle = WindowStyle::WINDOWED;
	m_EngineContext.GameSettings.Width = 1240;
	m_EngineContext.GameSettings.Height = 720;
	m_EngineContext.GameSettings.ClearColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_EngineContext.GameSettings.MSAA = true;
	m_EngineContext.GameSettings.Title = L"Nvidia Flex - Softbody Rendering";
	m_EngineContext.GameSettings.VerticalSync = true;
	m_EngineContext.GameSettings.UseDeferredRendering = false;
}

void GameManager::Initialize(EngineContext* pEngineContext)
{	
	flexInit(FLEX_VERSION, FlexHelper::FlexMessageCallback);

	m_pEngineContext = pEngineContext;

	//Add initial scene
	AddScene(new CollisionDemoScene());
	AddScene(new StressTestScene());
	AddScene(new ParticleScene());
	AddScene(new SoftBodyScene());
	AddScene(new DaeLogoScene());
	AddScene(new ParameterDemoScene());

	m_ActiveScene = 0;
	m_pScenes[m_ActiveScene]->BaseInitialize(pEngineContext);
	m_SceneInitialized = true;
}

void GameManager::Render()
{
	if(m_pScenes[m_ActiveScene] && m_pScenes[m_ActiveScene]->IsInitialized())
		m_pScenes[m_ActiveScene]->BaseRender();
}

void GameManager::Update()
{
	if (m_pScenes[m_ActiveScene])
	{
		if(m_pScenes[m_ActiveScene]->IsInitialized() == false)
			m_pScenes[m_ActiveScene]->BaseInitialize(m_pEngineContext);
		m_pScenes[m_ActiveScene]->BaseUpdate();
	}
}

void GameManager::OnResize()
{
	FluxCore::OnResize();
	if (m_ActiveScene >= 0 && m_ActiveScene < (int)m_pScenes.size() && m_pScenes[m_ActiveScene])
		m_pScenes[m_ActiveScene]->OnResize();
}

LRESULT GameManager::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		if ((int)wParam == VK_F5)
		{
			m_ActiveScene = (m_ActiveScene - 1 + m_pScenes.size()) % m_pScenes.size();
		}
		else if ((int)wParam == VK_F6)
		{
			m_ActiveScene = (m_ActiveScene + 1) % m_pScenes.size();
		}
		return 0;
	}

	return FluxCore::WndProc(hWnd, message, wParam, lParam);
}

void GameManager::AddScene(SceneBase* pScene)
{
	m_pScenes.push_back(pScene);
	m_ActiveScene = m_pScenes.size() - 1;
}

void GameManager::ReloadScene(SceneBase* pScene)
{
	SafeDelete(m_pScenes[m_ActiveScene]);
	m_pScenes[m_ActiveScene] = pScene;
}

void GameManager::LoadScene(const int index)
{
	if(index >= (int)m_pScenes.size())
	{
		DebugLog::LogFormat(LogType::WARNING, L"[GameManager::LoadScene()] > Scene with index '%i' does not exist!", index);
		return;
	}
	m_ActiveScene = index;
}