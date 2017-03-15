#include "stdafx.h"
#include "SkyboxMaterial.h"
#include "../Graphics/Texture.h"

SkyboxMaterial::SkyboxMaterial() : Material()
{
	m_MaterialDesc.EffectName = "./Resources/Shaders/Skybox.fx";
	m_MaterialDesc.HasWorldMatrix = true;
	m_MaterialDesc.HasWvpMatrix = true;
}
SkyboxMaterial::~SkyboxMaterial()
{
}

void SkyboxMaterial::LoadShaderVariables()
{
	BIND_AND_CHECK_NAME(m_pTextureVar, gSkybox, AsShaderResource);
}

void SkyboxMaterial::UpdateShaderVariables(MeshComponent* pMeshComponent)
{
	UNREFERENCED_PARAMETER(pMeshComponent);
	m_pTextureVar->SetResource(m_pTexture->GetResourceView());
}