#include "FluxEngine.h"
#include "Scene.h"
#include "SceneNode.h"
#include "Rendering\Renderer.h"
#include "Component.h"
#include "Rendering\Model.h"

Scene::Scene(Context* pContext) : SceneNode(pContext, this)
{
	m_pRenderer = pContext->GetSubsystem<Renderer>();
}

Scene::~Scene()
{
	for (SceneNode*& pNode : m_Nodes)
	{
		delete pNode;
	}
	m_Nodes.clear();
	for (Component*& pComponent : m_Components)
	{
		delete pComponent;
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

void Scene::TrackChild(SceneNode* pNode)
{
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

void Scene::OnSceneSet(Scene* /*pScene*/)
{
	SceneNode::OnSceneSet(this);
}

SceneNode* Scene::PickNode(const Ray& ray)
{
	std::vector<SceneNode*> sortedNodes = m_Nodes;
	std::sort(sortedNodes.begin(), sortedNodes.end(), [&ray](SceneNode* pA, SceneNode* pB) { return Vector3::DistanceSquared(pA->GetWorldPosition(), ray.position) < Vector3::DistanceSquared(pB->GetWorldPosition(), ray.position); });

	SceneNode* pClosest = nullptr;
	float minDistance = std::numeric_limits<float>::max();
	for (SceneNode* pNode : sortedNodes)
	{
		Drawable* pDrawable = pNode->GetComponent<Drawable>();
		if (pDrawable)
		{
			float distance;
			if (ray.Intersects(pDrawable->GetWorldBoundingBox(), distance))
			{
				if (distance < minDistance)
				{
					minDistance = distance;
					pClosest = pNode;
				}
			}
		}
	}
	return pClosest;
}
