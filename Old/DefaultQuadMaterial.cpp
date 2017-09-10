#include "stdafx.h"
#include "DefaultQuadMaterial.h"

DefaultQuadMaterial::DefaultQuadMaterial()
{
	m_MaterialDesc.EffectName = "./Resources/Shaders/Rendering/DefaultQuadRenderer.fx";
}

DefaultQuadMaterial::~DefaultQuadMaterial()
{
}

void DefaultQuadMaterial::SetTexture(ID3D11ShaderResourceView* pSRV)
{
	m_pEffect->GetVariableByName("gTexture")->AsShaderResource()->SetResource(pSRV);
}
