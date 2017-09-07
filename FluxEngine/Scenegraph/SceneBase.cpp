#include "stdafx.h"
#include "SceneBase.h"
#include "GameObject.h"
#include "../Components/Camera.h"
#include "../Prefabs/FreeCamera.h"
#include "../Rendering/DeferredRenderer.h"
#include "../Rendering/ShadowMapper.h"
#include "../Managers/MaterialManager.h"
#include "../Components/Transform.h"

SceneBase::SceneBase(const string& name) : m_SceneName(name)
{
}

SceneBase::~SceneBase()
{
	//Clean up all the gameobjects
	for (size_t i = 0; i < m_pChildren.size(); i++)
		SafeDelete(m_pChildren[i]);

	SafeDelete(m_SceneContext.ShadowMapper);
	SafeDelete(m_pDeferredRenderer);
	SafeDelete(m_SceneContext.MaterialManager);
}

void SceneBase::BaseInitialize(EngineContext* pEngineContext)
{
	if (m_Initialized) 
		return;

	PerfTimer timer("Scene Initialization");

	//Set general engine pointer
	m_GameContext.Engine = pEngineContext;
	m_GameContext.Scene = &m_SceneContext;
	m_pGameContext = &m_GameContext;

	//Add default camera
	FreeCamera* pCamera = new FreeCamera();
	AddChild(pCamera);
	m_GameContext.Scene->Camera = pCamera->GetCamera();
	pCamera->GetCamera()->SetActive(true);
	pCamera->GetTransform()->SetRotation(25.0f, 0, 0);
	pCamera->GetTransform()->Translate(0, 12, -20);

	m_SceneContext.ShadowMapper = new ShadowMapper();
	m_SceneContext.ShadowMapper->Initialize(m_pGameContext);

	m_SceneContext.MaterialManager = new MaterialManager();
	m_SceneContext.MaterialManager->Initialize(m_pGameContext);

	Initialize();

	if (m_pGameContext->Engine->GameSettings.UseDeferredRendering)
	{
		m_pDeferredRenderer = new DeferredRenderer();
		m_pDeferredRenderer->Initialize(m_pGameContext);
		m_pDeferredRenderer->CreateGBuffer();
	}
	m_Initialized = true;

	timer.Stop();
}

void SceneBase::BaseUpdate()
{
	Update();
	for (size_t i = 0; i < m_pChildren.size(); i++)
		m_pChildren[i]->BaseUpdate();
	LateUpdate();
}

void SceneBase::BaseRender()
{	
	/*m_SceneContext.ShadowMapper->SetLight(XMFLOAT3(0,0,0), XMFLOAT3(-0.577f, -0.577f, 0.577f));
	m_SceneContext.ShadowMapper->Begin();
	for (size_t i = 0; i < m_pChildren.size(); i++)
		m_SceneContext.ShadowMapper->Render(m_pChildren[i]);
	m_SceneContext.ShadowMapper->End();*/

	if(m_pDeferredRenderer)
		m_pDeferredRenderer->Begin();

	for (size_t i = 0; i < m_pChildren.size(); i++)
		m_pChildren[i]->BaseRender();

	Render();
}

void SceneBase::AddChild(GameObject* pChild)
{
	pChild->BaseInitialize(&m_GameContext);
	pChild->m_pScene = this;
	m_pChildren.push_back(pChild);
}

void SceneBase::OnResize()
{
	if (m_pDeferredRenderer)
		m_pDeferredRenderer->CreateGBuffer();
}

GameObject* SceneBase::FindObject(const string& name)
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