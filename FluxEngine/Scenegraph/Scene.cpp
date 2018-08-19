#include "FluxEngine.h"
#include "Scene.h"
#include "SceneNode.h"
#include "Rendering\Renderer.h"
#include "Component.h"

Scene::Scene(Context* pContext) : SceneNode(pContext, this)
{
	m_pRenderer = pContext->GetSubsystem<Renderer>();
}

Scene::~Scene()
{
	for (SceneNode*& pNode : m_Nodes)
	{
		SafeDelete(pNode);
	}
	m_Nodes.clear();
	for (Component*& pComponent : m_Components)
	{
		SafeDelete(pComponent);
	}
	m_Components.clear();
}

void Scene::Initialize()
{
	AUTOPROFILE(Scene_Initialize);
}

void Scene::Update()
{
	AUTOPROFILE(Scene_Update);

	{
		AUTOPROFILE(Scene_UpdateScene);
		m_OnSceneUpdate.Broadcast();
	}

	m_pRenderer->Draw();
}

void Scene::AddChild(SceneNode* pNode)
{
	pNode->OnSceneSet(this);
	m_Nodes.push_back(pNode);
}

Camera* Scene::GetCamera() const
{
	return m_pRenderer->GetCamera(0);
}

SceneNode* Scene::FindNode(const std::string& name)
{
	for (SceneNode* pNode : m_Nodes)
	{
		if (pNode->GetName() == name)
		{
			return pNode;
		}
	}
	return nullptr;
}

void Scene::OnSceneSet(Scene* pScene)
{
	UNREFERENCED_PARAMETER(pScene);
	SceneNode::OnSceneSet(this);
}