#include "stdafx.h"
#include "DemoScene.h"
#include "../../Components/MeshComponent.h"
#include "../../Scenegraph/GameObject.h"
#include "../../Materials/Forward/DefaultMaterial.h"
#include "../../Components/TransformComponent.h"
#include "../../Components/CameraComponent.h"

DemoScene::DemoScene()
{}

DemoScene::~DemoScene()
{
}

void DemoScene::Initialize()
{
	m_pGameObject = new GameObject();
	MeshComponent* pMesh = new MeshComponent("./Resources/Meshes/Player.flux");
	DefaultMaterial* pMaterial = new DefaultMaterial();
	pMaterial->Initialize(m_pGameContext);
	pMesh->SetMaterial(pMaterial);
	m_pGameObject->AddComponent(pMesh);
	
	AddChild(m_pGameObject);
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
