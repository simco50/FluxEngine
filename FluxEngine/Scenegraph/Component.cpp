#include "FluxEngine.h"
#include "Component.h"
#include "SceneNode.h"

Component::Component(Context* pContext) :
	Object(pContext)
{

}

Component::~Component()
{

}

void Component::OnSceneSet(Scene* pScene)
{
	if (m_pScene)
	{
		FLUX_LOG(Error, "[Component::OnSceneSet] > Component already has a scene assigned!");
		return;
	}
	m_pScene = pScene;
}

void Component::OnNodeSet(SceneNode* pNode)
{
	if (m_pNode)
	{
		FLUX_LOG(Error, "[Component::OnNodeSet] > Component already has a node assigned!");
		return;
	}
	m_pNode = pNode;
}

void Component::OnNodeRemoved()
{
	m_pNode = nullptr;
}

void Component::OnSceneRemoved()
{
	m_pScene = nullptr;
}

Component* Component::GetComponent(StringHash type) const
{
	checkf(m_pNode, "[Component::GetComponent] > Component is not attached to a SceneNode");
	return m_pNode->GetComponent(type);
}
