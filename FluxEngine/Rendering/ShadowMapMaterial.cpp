#include "stdafx.h"
#include "ShadowMapMaterial.h"


ShadowMapMaterial::ShadowMapMaterial() : Material()
{
	m_MaterialDesc.EffectName = "./Resources/Shaders/ShadowMap.fx";
	m_MaterialDesc.HasWorldMatrix = true;
}

ShadowMapMaterial::~ShadowMapMaterial()
{
}

void ShadowMapMaterial::SetLightVP(const XMFLOAT4X4& matrix)
{
	m_pLightVPV->SetMatrix(reinterpret_cast<const float*>(&matrix));
}

void ShadowMapMaterial::LoadShaderVariables()
{
	BIND_AND_CHECK_NAME(m_pLightVPV, gLightVP, AsMatrix);
}

void ShadowMapMaterial::UpdateShaderVariables(MeshComponent* pMeshComponent)
{
	UNREFERENCED_PARAMETER(pMeshComponent);
}
