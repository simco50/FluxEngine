#include "stdafx.h"
#include "GameManager.h"
#include "../Scenegraph/SceneBase.h"
#include "../Game/SoftBodyScene.h"

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
	m_EngineContext.GameSettings.MSAA = false;
	m_EngineContext.GameSettings.Title = L"FluxEngine v0.9";
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
		CalculateFrameStats();
	}
}

void GameManager::OnPause(const bool paused)
{
	if(m_pActiveScene)
	{
		if(paused)
			m_pActiveScene->m_SceneContext.GameTimer.Stop();
		else
			m_pActiveScene->m_SceneContext.GameTimer.Start();
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

void GameManager::CalculateFrameStats() const
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	if (m_SceneInitialized == false)
		return;

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((m_pActiveScene->GetSceneContext()->GameTimer.GameTime() - timeElapsed) >= 1.0f)
	{
		int fps = frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / (float)fps;

		wstring fpsStr = to_wstring(fps);
		wstring mspfStr = to_wstring(mspf);

		wstring windowText = m_EngineContext.GameSettings.Title +
			L"\t FPS: " + fpsStr +
			L"\t MSPF: " + mspfStr;

		SetWindowText(m_EngineContext.Hwnd, windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}
