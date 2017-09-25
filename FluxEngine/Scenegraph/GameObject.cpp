#include "stdafx.h"
#include "GameObject.h"
#include "Core/Components/ComponentBase.h"
#include "Core/Components/Transform.h"

GameObject::GameObject()
{
	m_pTransform = new Transform();
	AddComponent(m_pTransform);
}

GameObject::~GameObject()
{
	for (size_t i = 0; i < m_pChildren.size(); i++)
		delete m_pChildren[i];
	for (size_t i = 0; i < m_pComponents.size(); i++)
		delete m_pComponents[i];
}

void GameObject::AddComponent(ComponentBase* pComponent)
{
	for (ComponentBase* pComp : m_pComponents)
	{
		if (pComponent == pComp)
		{
			FLUX_LOG(WARNING, "GameObject::AddComponent() > Cannot add the same component twice!");
			return;
		}
	}

	pComponent->m_pGameObject = this;
	m_pComponents.push_back(pComponent);

	if(m_IsInitialized)
		pComponent->BaseInitialize(m_pGameContext);

}

void GameObject::AddChild(GameObject* pChild)
{
	pChild->m_pParent = this;
	m_pChildren.push_back(pChild);

	if (m_IsInitialized)
		pChild->BaseInitialize(m_pGameContext);
}

Transform* GameObject::GetTransform()
{
	return m_pTransform;
}

GameObject* GameObject::Find(const string& name)
{
	for(GameObject* pChild : m_pChildren)
	{
		if (pChild->GetName() == name)
			return pChild;
		GameObject* pChildOutput = pChild->Find(name);
		if (pChildOutput != nullptr)
			return pChildOutput;
	}
	return nullptr;
}

SceneBase* GameObject::GetScene()
{
	GameObject* pParent = m_pParent;
	while(pParent != nullptr)
		pParent = pParent->GetParent();
	return pParent->m_pScene;
}

void GameObject::BaseInitialize(GameContext* pGameContext)
{
	if (m_IsInitialized)
		return;

	m_pGameContext = pGameContext;

	Initialize();

	for (size_t i = 0; i < m_pChildren.size(); i++)
		m_pChildren[i]->BaseInitialize(pGameContext);
	for (size_t i = 0; i < m_pComponents.size(); i++)
		m_pComponents[i]->BaseInitialize(pGameContext);

	m_IsInitialized = true;
}

void GameObject::BaseUpdate()
{
	Update();
	for (size_t i = 0; i < m_pChildren.size(); i++)
		m_pChildren[i]->BaseUpdate();
	for (size_t i = 0; i < m_pComponents.size(); i++)
		m_pComponents[i]->Update();
}