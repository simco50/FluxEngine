#include "stdafx.h"
#include "Skybox.h"
#include "../Components/MeshRenderer.h"
#include "../Materials/SkyboxMaterial.h"

Skybox::Skybox()
{
}


Skybox::~Skybox()
{
}

void Skybox::Initialize()
{
	MeshRenderer* pMesh = new MeshRenderer(ResourceManager::Load<MeshFilter>("./Resources/Meshes/Cube.flux"));
	AddComponent(pMesh);
	m_pMaterial = make_unique<SkyboxMaterial>();
	m_pMaterial->Initialize(m_pGameContext);
	pMesh->SetMaterial(m_pMaterial.get());
	pMesh->SetFrustumCulling(false);
}

void Skybox::Update()
{
}

void Skybox::Render()
{
}

void Skybox::SetTexture(Texture* pTexture)
{
	m_pMaterial->SetTexture(pTexture);
}
