#include "stdafx.h"
#include "GBufferMaterial.h"


GBufferMaterial::GBufferMaterial() : Material()
{
	m_MaterialDesc.EffectName = "./Resources/Shaders/Rendering/GBuffer.fx";
}


GBufferMaterial::~GBufferMaterial()
{
}

void GBufferMaterial::SetDiffuseSRV(ID3D11ShaderResourceView* pSRV)
{
	if(m_pDiffuseSRV)
		m_pDiffuseSRV->SetResource(pSRV);
}

void GBufferMaterial::SetNormalSRV(ID3D11ShaderResourceView* pSRV)
{
	if(m_pNormalSRV)
		m_pNormalSRV->SetResource(pSRV);
}

void GBufferMaterial::SetDepthSRV(ID3D11ShaderResourceView* pSRV)
{
	if (m_pDepthSRV)
		m_pDepthSRV->SetResource(pSRV);
}

void GBufferMaterial::SetEyePos(const XMFLOAT3& eyePos)
{
	if(m_pEyePosVar)
		m_pEyePosVar->SetFloatVector(reinterpret_cast<const float*>(&eyePos));
}

void GBufferMaterial::SetViewProjInv(const XMFLOAT4X4& viewProjInv)
{
	if(m_pViewProjInvVar)
		m_pViewProjInvVar->SetMatrix(reinterpret_cast<const float*>(&viewProjInv));
}

void GBufferMaterial::SetLightVP(const XMFLOAT4X4& vp)
{
	if (m_pLightVP)
		m_pLightVP->SetMatrix(reinterpret_cast<const float*>(&vp));
}

void GBufferMaterial::SetLightDirection(const XMFLOAT3& direction)
{
	if (m_pLightDirection)
		m_pLightDirection->SetFloatVector(reinterpret_cast<const float*>(&direction));
}

void GBufferMaterial::SetShadowMap(ID3D11ShaderResourceView* pSRV)
{
	if (m_pShadowMap)
		m_pShadowMap->SetResource(pSRV);
}

void GBufferMaterial::LoadShaderVariables()
{
	BIND_AND_CHECK_NAME(m_pDiffuseSRV, gDiffuse, AsShaderResource);
	BIND_AND_CHECK_NAME(m_pNormalSRV, gNormal, AsShaderResource);
	BIND_AND_CHECK_NAME(m_pDepthSRV, gDepth, AsShaderResource);
	BIND_AND_CHECK_NAME(m_pEyePosVar, gEyePos, AsVector);
	BIND_AND_CHECK_NAME(m_pViewProjInvVar, gViewProjInv, AsMatrix);

	BIND_AND_CHECK_NAME(m_pLightVP, gLightVP, AsMatrix);
	BIND_AND_CHECK_NAME(m_pShadowMap, gShadowMap, AsShaderResource);
	BIND_AND_CHECK_NAME(m_pLightDirection, gLightDirection, AsVector);
}

void GBufferMaterial::UpdateShaderVariables()
{
}
