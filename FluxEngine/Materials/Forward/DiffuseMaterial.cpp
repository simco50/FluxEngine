#include "stdafx.h"
#include "DiffuseMaterial.h"
#include "../Graphics/Texture.h"

DiffuseMaterial::DiffuseMaterial() : Material()
{
	m_MaterialDesc.EffectName = L"./Resources/Shaders/PosNormTex3D.fx";
	m_MaterialDesc.HasWorldMatrix = true;
	m_MaterialDesc.HasWvpMatrix = true;
}


DiffuseMaterial::~DiffuseMaterial()
{
}

void DiffuseMaterial::SetTexture(Texture* pTexture)
{
	m_pDiffuseTextureVariable->SetResource(pTexture->GetResourceView());
}

void DiffuseMaterial::LoadShaderVariables()
{
	BIND_AND_CHECK_NAME(m_pDiffuseTextureVariable, gDiffuseMap, AsShaderResource);
}

void DiffuseMaterial::UpdateShaderVariables(MeshComponent* pMeshComponent)
{
	UNREFERENCED_PARAMETER(pMeshComponent);
}
