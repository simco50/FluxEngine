#include "stdafx.h"
#include "BasicMaterial_Deferred.h"
#include "../../Graphics/Texture.h"

BasicMaterial_Deferred::BasicMaterial_Deferred() :Material()
{
	m_MaterialDesc.EffectName = "./Resources/Shaders/BasicMaterial_Deferred.fx";
	m_MaterialDesc.HasWorldMatrix = true;
	m_MaterialDesc.HasWvpMatrix = true;
}

BasicMaterial_Deferred::~BasicMaterial_Deferred()
{
}

void BasicMaterial_Deferred::LoadShaderVariables()
{
	BIND_AND_CHECK_NAME(m_pDiffuseTexture, gDiffuseMap, AsShaderResource);
	BIND_AND_CHECK_NAME(m_pUseDiffuseTexture, gUseDiffuseMap, AsScalar);
	BIND_AND_CHECK_NAME(m_pNormalTexture, gNormalMap, AsShaderResource);
	BIND_AND_CHECK_NAME(m_pUseNormalTexture, gUseNormalMap, AsScalar);
}

void BasicMaterial_Deferred::UpdateShaderVariables()
{
	m_pUseDiffuseTexture->SetBool(m_UseDiffuseTexture);
	m_pUseNormalTexture->SetBool(m_UseNormalMap);
	if(m_UseDiffuseTexture)
		m_pDiffuseTexture->SetResource(m_pDiffuseMap->GetResourceView());
	if(m_UseNormalMap)
		m_pNormalTexture->SetResource(m_pNormalMap->GetResourceView());
}

void BasicMaterial_Deferred::SetDiffuseTexture(Texture* pTexture)
{
	m_UseDiffuseTexture = true;
	m_pDiffuseMap = pTexture;
}

void BasicMaterial_Deferred::SetNormalTexture(Texture* pTexture)
{
	m_UseNormalMap = true;
	m_pNormalMap = pTexture;
}
