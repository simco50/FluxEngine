#include "FluxEngine.h"
#include "Scene.h"
#include "SceneNode.h"
#include "Rendering\Renderer.h"
#include "Component.h"

Scene::Scene(Context* pContext) : SceneNode(pContext, this)
{
	m_pRenderer = make_unique<Renderer>(pContext);
}

Scene::~Scene()
{
	for (SceneNode*& pNode : m_pNodes)
		SafeDelete(pNode);
	for (Component*& pComponent : m_Components)
		SafeDelete(pComponent);
}

void Scene::Initialize()
{
	AUTOPROFILE(Scene_Initialize);
}

void Scene::Update()
{
	SceneNode::Update();

	for (SceneNode* pNode : m_pNodes)
		pNode->Update();

	m_pRenderer->Draw();
}

void Scene::AddChild(SceneNode* pNode)
{
	pNode->OnSceneSet(this);
	m_pNodes.push_back(pNode);
}

Camera* Scene::GetCamera() const
{
	return m_pRenderer->GetCamera(0);
}

SceneNode* Scene::FindNode(const std::string& name)
{
	for (SceneNode* pNode : m_pNodes)
	{
		if (pNode->GetName() == name)
			return pNode;
	}
	return nullptr;
}

void Scene::OnSceneSet(Scene* pScene)
{
	UNREFERENCED_PARAMETER(pScene);
	SceneNode::OnSceneSet(this);
}