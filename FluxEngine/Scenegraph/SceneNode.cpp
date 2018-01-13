#include "FluxEngine.h"
#include "SceneNode.h"
#include "Scene.h"
#include "Component.h"
#include "Transform.h"

SceneNode::SceneNode() : SceneNode("")
{
	
}

SceneNode::SceneNode(const std::string& name) : 
	m_Name(name)
{
	m_pTransform = make_unique<Transform>(this);
	m_pTransform->Initialize();
}

SceneNode::~SceneNode()
{
	for (Component*& pComponent : m_Components)
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

	//The component don't have the scene assigned yet
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

	//If the node is already added to the scene
	pComponent->OnNodeSet(this);
	if (m_pScene)
		pComponent->OnSceneSet(m_pScene);
}

Component* SceneNode::GetComponent(StringHash type)
{
	for (Component* pComponent : m_Components)
	{
		if (type == pComponent->GetType())
			return pComponent;
	}
	return nullptr;
}
