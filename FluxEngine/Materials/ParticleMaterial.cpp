#include "stdafx.h"
#include "ParticleMaterial.h"
#include "Rendering/ParticleSystem\ParticleSystem.h"
#include "Rendering/Core/Texture.h"
#include "Rendering/Camera/Camera.h"

ParticleMaterial::ParticleMaterial() : Material()
{
	m_MaterialDesc.EffectName = "./Resources/Shaders/ParticleRenderer.fx";
	m_MaterialDesc.HasWorldMatrix = true;
	m_MaterialDesc.HasViewInverseMatrix = true;
}

ParticleMaterial::~ParticleMaterial()
{
}

void ParticleMaterial::LoadShaderVariables()
{
	BIND_AND_CHECK_NAME(m_pTextureVariable, gParticleTexture, AsShaderResource);
	BIND_AND_CHECK_SEMANTIC(m_pViewProjVariable, "ViewProj", AsMatrix);

	m_pAdditiveBlendingTechnique = m_pEffect->GetTechniqueByIndex(BlendMode::ADDITIVE);
	m_pAlphaBlendingTechnique = m_pEffect->GetTechniqueByIndex(BlendMode::ALPHABLEND);
}

void ParticleMaterial::UpdateShaderVariables()
{
	m_pTextureVariable->SetResource((ID3D11ShaderResourceView*)m_pTexture->GetResourceView());
	XMFLOAT4X4 vp = m_pGameContext->Scene->Camera->GetViewProjection();
	m_pViewProjVariable->SetMatrix(reinterpret_cast<float*>(&vp));
}

ID3DX11EffectTechnique* ParticleMaterial::GetTechnique() const
{
	switch (m_BlendMode)
	{
	case ALPHABLEND:
		return m_pAlphaBlendingTechnique;
	case ADDITIVE:
		return m_pAdditiveBlendingTechnique;
	default:
		return nullptr;
	}
}

void ParticleMaterial::SetBlendMode(const BlendMode mode)
{
	m_BlendMode = mode;
}

void ParticleMaterial::SetTexture(const string& path)
{
	m_pTexture = ResourceManager::Load<Texture>(path);
}
