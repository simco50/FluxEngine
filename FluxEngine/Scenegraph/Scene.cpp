#include "stdafx.h"
#include "Scene.h"
#include "SceneNode.h"
#include "Rendering\Drawable.h"
#include "Rendering\Geometry.h"
#include "Rendering\Renderer.h"

Scene::Scene(Graphics* pGraphics)
{
	m_pRenderer = make_unique<Renderer>(pGraphics);
}


Scene::~Scene()
{
	for (SceneNode*& pNode : m_pNodes)
		SafeDelete(pNode);
}

void Scene::Initialize()
{

}

void Scene::Update()
{
	for (SceneNode* pNode : m_pNodes)
		pNode->Update();

	m_pRenderer->Draw();
}

void Scene::AddChild(SceneNode* pNode)
{
	pNode->OnSceneSet(this);
	m_pNodes.push_back(pNode);
}