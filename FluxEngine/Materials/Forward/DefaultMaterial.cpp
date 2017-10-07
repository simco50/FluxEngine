#include "stdafx.h"
#include "DefaultMaterial.h"
#include "Rendering/Core/Texture.h"

ID3DX11EffectVectorVariable* DefaultMaterial::m_pColorVar = nullptr;
ID3DX11EffectShaderResourceVariable* DefaultMaterial::m_pDiffuseTextureVar = nullptr;
ID3DX11EffectScalarVariable* DefaultMaterial::m_pUseDiffuseTextureVar = nullptr;

DefaultMaterial::DefaultMaterial() : Material()
{
	m_MaterialDesc.EffectName = "./Resources/Shaders/Forward/Default_Forward.fx";
	m_MaterialDesc.HasWorldMatrix = true;
	m_MaterialDesc.HasWvpMatrix = true;
}

DefaultMaterial::~DefaultMaterial()
{
}

void DefaultMaterial::LoadShaderVariables()
{
	BIND_AND_CHECK_NAME(m_pColorVar, gColor, AsVector);
	m_pColorVar->GetFloatVector(&m_Color.x);
	BIND_AND_CHECK_NAME(m_pDiffuseTextureVar, gDiffuseTexture, AsShaderResource);
	BIND_AND_CHECK_NAME(m_pUseDiffuseTextureVar, gUseDiffuseTexture, AsScalar);
}

void DefaultMaterial::UpdateShaderVariables()
{
	m_pColorVar->SetFloatVector(&m_Color.x);
	m_pUseDiffuseTextureVar->SetBool(m_pDiffuseTexture == nullptr ? false : true);
	/*if(m_pDiffuseTexture)
		m_pDiffuseTextureVar->SetResource(m_pDiffuseTexture->GetResourceView());*/
}

void DefaultMaterial::SetDiffuseTexture(const string& filePath)
{
	m_pDiffuseTexture = ResourceManager::Load<Texture>(filePath);
}
