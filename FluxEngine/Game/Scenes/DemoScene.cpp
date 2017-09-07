#include "stdafx.h"
#include "DemoScene.h"
#include "../../Components/MeshRenderer.h"
#include "../../Scenegraph/GameObject.h"
#include "../../Materials/Forward/DefaultMaterial.h"
#include "../../Components/Transform.h"
#include "../../Components/Camera.h"

DemoScene::DemoScene()
{}

DemoScene::~DemoScene()
{
}

void DemoScene::Initialize()
{
	DefaultMaterial* pMat = new DefaultMaterial();
	pMat->Initialize(m_pGameContext);

	GameObject* pObj = new GameObject();
	MeshRenderer* pMeshRenderer = new MeshRenderer(ResourceManager::Load<MeshFilter>("Resources/Meshes/bust.flux"));
	pMeshRenderer->SetMaterial(pMat);

	pObj->AddComponent(pMeshRenderer);
	AddChild(pObj);
}

void DemoScene::Update()
{
}

void DemoScene::LateUpdate()
{
}

void DemoScene::Render()
{
}