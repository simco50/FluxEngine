#include "stdafx.h"
#include "SceneNode.h"
#include "Scene.h"
#include "Component.h"
#include "Core/Components/Transform.h"

SceneNode::SceneNode()
{
	m_pTransform = make_unique<Transform>(this);
	m_pTransform->Initialize();
}

SceneNode::~SceneNode()
{
	for (Component* pComponent : m_Components)
		SafeDelete(pComponent);
}

void SceneNode::Update()
{
	m_pTransform->Update();
	for (Component* pComponent : m_Components)
		pComponent->Update();
}

void SceneNode::OnSceneSet(Scene* pScene)
{
	m_pScene = pScene;
	for (Component* pComponent : m_Components)
		pComponent->OnSceneSet(pScene);
}

void SceneNode::AddChild(SceneNode* pNode)
{
	m_pScene->AddChild(pNode);
	pNode->m_pParent = this;
}

void SceneNode::AddComponent(Component* pComponent)
{
	m_Components.push_back(pComponent);
	pComponent->OnNodeSet(this);
}
