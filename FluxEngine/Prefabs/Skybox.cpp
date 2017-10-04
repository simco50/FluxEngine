#include "stdafx.h"
#include "Skybox.h"
#include "Rendering/MeshRenderer.h"
#include "Materials/SkyboxMaterial.h"

Skybox::Skybox()
{
}


Skybox::~Skybox()
{
}

void Skybox::Initialize()
{
	MeshRenderer* pMesh = new MeshRenderer(ResourceManager::Load<MeshFilter>("FluxEngine/Resources/Meshes/Cube.flux"));
	AddComponent(pMesh);
	m_pMaterial = make_unique<SkyboxMaterial>();
	m_pMaterial->Initialize(m_pGameContext);
	pMesh->SetMaterial(m_pMaterial.get());
	pMesh->SetFrustumCulling(false);
}

void Skybox::Update()
{
}

void Skybox::SetTexture(Texture* pTexture)
{
	m_pMaterial->SetTexture(pTexture);
}
