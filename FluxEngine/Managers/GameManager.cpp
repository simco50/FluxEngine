#include "stdafx.h"
#include "GameManager.h"
#include "../Scenegraph/SceneBase.h"
#include "../Game/SoftBodyScene.h"
//#include "../Game/ParticleScene.h"

GameManager::GameManager()
{
}

GameManager::~GameManager()
{
	SafeDelete(m_pActiveScene);
}

void GameManager::PrepareGame()
{
	m_EngineContext.GameSettings.WindowStyle = WindowStyle::WINDOWED;
	m_EngineContext.GameSettings.Width = 1240;
	m_EngineContext.GameSettings.Height = 720;
	m_EngineContext.GameSettings.ClearColor = (XMFLOAT4)Colors::SteelBlue;
	m_EngineContext.GameSettings.MSAA = true;
	m_EngineContext.GameSettings.Title = L"FluxEngine v0.9.1";
	m_EngineContext.GameSettings.VerticalSync = true;
	m_EngineContext.GameSettings.UseDeferredRendering = false;
}

void GameManager::Initialize(EngineContext* pEngineContext)
{	
	m_pEngineContext = pEngineContext;

	//Add initial scene
	m_pActiveScene = new SoftBodyScene();
	m_pActiveScene->BaseInitialize(pEngineContext);
	m_SceneInitialized = true;
}

void GameManager::Render()
{
	if(m_pActiveScene && m_SceneInitialized)
		m_pActiveScene->BaseRender();
}

void GameManager::Update()
{
	if(m_pOldScene)
	{
		delete m_pOldScene;
		m_pOldScene = nullptr;
	}

	if (m_pActiveScene)
	{
		if(m_SceneInitialized == false)
		{
			m_pActiveScene->BaseInitialize(m_pEngineContext);
			m_SceneInitialized = true;
		}
		m_pActiveScene->BaseUpdate();
	}
}

void GameManager::OnResize()
{
	FluxCore::OnResize();
	if (m_pActiveScene)
		m_pActiveScene->OnResize();
}

void GameManager::LoadScene(SceneBase* pScene)
{
	m_pOldScene = m_pActiveScene;
	m_SceneInitialized = false;
	m_pActiveScene = pScene;
}

void GameManager::LoadSceneClean(SceneBase* pScene)
{
	ResourceManager::Reset();
	LoadScene(pScene);
}