#include "stdafx.h"
#include "DemoScene.h"
#include "Rendering/MeshRenderer.h"
#include "Scenegraph/GameObject.h"
#include "Materials/Forward/DefaultMaterial.h"
#include "Core/Components/Transform.h"
#include "Rendering/Camera/Camera.h"
#include "Rendering/ParticleSystem/ParticleSystem.h"
#include "Rendering/ParticleSystem/ParticleEmitter.h"

DemoScene::DemoScene()
{}

DemoScene::~DemoScene()
{
}

void DemoScene::Initialize()
{
	DefaultMaterial* pMat = new DefaultMaterial();
	pMat->Initialize(m_pGameContext);
	pMat->SetDiffuseTexture("./Resources/Textures/8ilFggp.jpg");

	MeshFilter* pMesh = ResourceManager::Load<MeshFilter>("Resources/Meshes/bust.flux");

	for (int x = 0; x < 5; ++x)
	{
		for (int y = 0; y < 1; ++y)
		{
			for (int z = 0; z < 1; ++z)
			{
				GameObject* pObj = new GameObject();
				MeshRenderer* pMeshRenderer = new MeshRenderer(pMesh);
				pMeshRenderer->SetMaterial(pMat);

				pObj->AddComponent(pMeshRenderer);
				AddChild(pObj);
				pObj->GetTransform()->SetScale(0.1f);
				pObj->GetTransform()->SetPosition(x * 7.0f, y * 7.0f, z * 7.0f);

				pObj->GetTransform()->Rotate(0.0f, 0.0f, z % 10 * 20);

				objects.push_back(pObj);
			}
		}
	}

	/*for (int x = 0; x < 5; ++x)
	{
		for (int z = 0; z < 5; ++z)
		{
			GameObject* pObj = new GameObject();
			ParticleSystem* pSystem = ResourceManager::Load<ParticleSystem>("Resources/ParticleSystems/Iris.json");
			ParticleEmitter* pEmitter = new ParticleEmitter(pSystem);
			pObj->AddComponent(pEmitter);
			AddChild(pObj);
			pObj->GetTransform()->SetPosition(x * 7.0f, 0, z * 7.0f);
		}
	}*/
}

void DemoScene::Update()
{
	for (int i = 0; i < objects.size(); ++i)
	{
		objects[i]->GetTransform()->Rotate(0, 0, pow(-1, i) * 2);
	}
}

void DemoScene::LateUpdate()
{
}