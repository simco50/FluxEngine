#include "stdafx.h"
#include "SceneBase.h"
#include "GameObject.h"
#include "Rendering/Camera/Camera.h"
#include "Rendering/Camera/FreeCamera.h"
#include "Core/Components/Transform.h"

SceneBase::SceneBase(const string& name) : m_SceneName(name)
{
}

SceneBase::~SceneBase()
{
	//Clean up all the gameobjects
	for (size_t i = 0; i < m_pChildren.size(); i++)
		SafeDelete(m_pChildren[i]);
}

void SceneBase::BaseInitialize(EngineContext* pEngineContext)
{
	if (m_Initialized) 
		return;

	AUTOPROFILE(Scene);

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

	Initialize();

	m_Initialized = true;
}

void SceneBase::BaseUpdate()
{
	Update();
	for (size_t i = 0; i < m_pChildren.size(); i++)
		m_pChildren[i]->BaseUpdate();
	LateUpdate();
}

void SceneBase::AddChild(GameObject* pChild)
{
	pChild->BaseInitialize(&m_GameContext);
	pChild->m_pScene = this;
	m_pChildren.push_back(pChild);
}

void SceneBase::OnResize()
{
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