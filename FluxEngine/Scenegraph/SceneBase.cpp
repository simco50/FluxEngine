#include "stdafx.h"
#include "SceneBase.h"
#include "GameObject.h"
#include "../Components/CameraComponent.h"
#include "../Prefabs/FreeCamera.h"
#include "../Rendering/DeferredRenderer.h"
#include "../Rendering/ShadowMapper.h"

SceneBase::SceneBase()
{
}

SceneBase::~SceneBase()
{
	//Clean up all the gameobjects
	for (size_t i = 0; i < m_pChildren.size(); i++)
		SafeDelete(m_pChildren[i]);

	SafeDelete(m_GameContext.Scene->Input);
	SafeDelete(m_SceneContext.ShadowMapper);
	SafeDelete(m_pDeferredRenderer);
}

void SceneBase::BaseInitialize(EngineContext* pEngineContext)
{
	PerfTimer timer(L"Scene Initialization");

	//Set general engine pointer
	m_GameContext.Engine = pEngineContext;
	m_GameContext.Scene = &m_SceneContext;
	m_pGameContext = &m_GameContext;

	//Add default camera
	FreeCamera* pCamera = new FreeCamera();
	AddChild(pCamera);
	m_GameContext.Scene->Cameras.push_back(pCamera->GetCamera());
	pCamera->GetCamera()->SetActive(true);

	//Set scene specific objects
	m_SceneContext.GameTimer.Reset();
	m_SceneContext.Input = new InputManager();
	m_SceneContext.Input->Initialize();

	m_SceneContext.ShadowMapper = new ShadowMapper();
	m_SceneContext.ShadowMapper->Initialize(m_pGameContext);

	Initialize();

	if (m_pGameContext->Engine->GameSettings.UseDeferredRendering)
	{
		m_pDeferredRenderer = new DeferredRenderer();
		m_pDeferredRenderer->Initialize(m_pGameContext);
		m_pDeferredRenderer->CreateGBuffer();
	}

	timer.Stop();
}

void SceneBase::BaseUpdate()
{
	//Update scene specific objects
	m_SceneContext.Input->Update();
	m_SceneContext.GameTimer.Tick();

	Update();
	for (size_t i = 0; i < m_pChildren.size(); i++)
		m_pChildren[i]->BaseUpdate();
	LateUpdate();
}

void SceneBase::BaseRender()
{	
	for (CameraComponent* pCamera : m_pGameContext->Scene->Cameras)
	{
		m_pGameContext->Engine->D3DeviceContext->RSSetViewports(1, &pCamera->GetViewport());
		m_pGameContext->Scene->CurrentCamera = pCamera;

		/*m_SceneContext.ShadowMapper->SetLight(XMFLOAT3(0,0,0), XMFLOAT3(-0.577f, -0.577f, 0.577f));
		m_SceneContext.ShadowMapper->Begin();
		for (size_t i = 0; i < m_pChildren.size(); i++)
			m_SceneContext.ShadowMapper->Render(m_pChildren[i]);
		m_SceneContext.ShadowMapper->End();*/

		if(m_pDeferredRenderer)
			m_pDeferredRenderer->Begin();

		for (size_t i = 0; i < m_pChildren.size(); i++)
			m_pChildren[i]->BaseRender();

		if(m_pDeferredRenderer)
			m_pDeferredRenderer->End();

		Render();
	}
}

void SceneBase::AddChild(GameObject* pChild)
{
	pChild->BaseInitialize(&m_GameContext);
	pChild->m_pScene = this;
	m_pChildren.push_back(pChild);
}

void SceneBase::SetPaused(const bool paused)
{
	if (paused == m_Paused)
		return;
	m_Paused = paused;
	if (paused)
		m_pGameContext->Scene->GameTimer.Stop();
	else
		m_pGameContext->Scene->GameTimer.Start();
}

void SceneBase::OnResize()
{
	if (m_pDeferredRenderer)
		m_pDeferredRenderer->CreateGBuffer();

	for (CameraComponent *pCamera : m_pGameContext->Scene->Cameras)
		pCamera->UpdateViewport();
}

GameObject* SceneBase::FindObject(const wstring& name)
{
	for (GameObject* pObject : m_pChildren)
	{
		if (pObject->GetName() == name)
			return pObject;
		GameObject* pChildObject = pObject->Find(name);
		if (pChildObject != nullptr)
			return pChildObject;
	}
	return nullptr;
}