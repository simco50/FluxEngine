#include "FluxEngine.h"
#include "SceneNode.h"
#include "Scene.h"
#include "Component.h"
#include "Transform.h"

SceneNode::SceneNode(Context* pContext, const std::string& name) :
	Object(pContext),
	m_Name(name)
{
	m_pTransform = std::make_unique<Transform>(this);
	m_pTransform->Initialize();
}

SceneNode::SceneNode(Context* pContext, Scene* pScene) :
	Object(pContext),
	m_pScene(pScene)
{
	m_pTransform = std::make_unique<Transform>(this);
	m_pTransform->Initialize();
}

SceneNode::~SceneNode()
{
	for (Component*& pComponent : m_Components)
	{
		delete pComponent;
	}
	m_Components.clear();
}

void SceneNode::OnSceneSet(Scene* pScene)
{
	m_pScene = pScene;

	//The component don't have the scene assigned yet
	for (Component* pComponent : m_Components)
	{
		pComponent->OnSceneSet(pScene);
	}
}

void SceneNode::OnSceneRemoved()
{
	m_pScene = nullptr;

	for (Component* pComponent : m_Components)
	{
		pComponent->OnSceneRemoved();
	}
}

SceneNode* SceneNode::CreateChild(const std::string& name)
{
	SceneNode* pNode = new SceneNode(m_pContext, name);
	AddChild(pNode);
	return pNode;
}

void SceneNode::AddChild(SceneNode* pNode)
{
	m_pScene->AddChild(pNode);
	pNode->m_pParent = this;
}

void SceneNode::AddComponent(Component* pComponent)
{
	if (GetComponent(pComponent->GetType()) != nullptr && !pComponent->CanHaveMultiple())
	{
		FLUX_LOG(Error, "[SceneNode::AddComponent] > SceneNode already has a %s", pComponent->GetTypeName().c_str());
		return;
	}

	m_Components.push_back(pComponent);

	//If the node is already added to the scene
	pComponent->OnNodeSet(this);
	if (m_pScene)
	{
		pComponent->OnSceneSet(m_pScene);
	}
}

Component* SceneNode::GetComponent(StringHash type)
{
	for (Component* pComponent : m_Components)
	{
		if (pComponent->IsTypeOf(type))
		{
			return pComponent;
		}
	}
	return nullptr;
}

void SceneNode::OnTransformDirty(const Transform* pTransform)
{
	for (Component* pComponent : m_Components)
	{
		pComponent->OnMarkedDirty(pTransform);
	}
}
