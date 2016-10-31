#include "stdafx.h"
#include "DefaultMaterial.h"
#include "../../Graphics/Texture.h"

ID3DX11EffectVectorVariable* DefaultMaterial::m_pColorVar = nullptr;
ID3DX11EffectShaderResourceVariable* DefaultMaterial::m_pDiffuseTextureVar = nullptr;
ID3DX11EffectScalarVariable* DefaultMaterial::m_pUseDiffuseTextureVar = nullptr;

DefaultMaterial::DefaultMaterial() : Material()
{
	m_MaterialDesc.EffectName = L"./Resources/Shaders/Forward/Default_Forward.fx";
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

void DefaultMaterial::UpdateShaderVariables(MeshComponent* pMeshComponent)
{
	UNREFERENCED_PARAMETER(pMeshComponent);
	m_pColorVar->SetFloatVector(&m_Color.x);
	m_pUseDiffuseTextureVar->SetBool(m_pDiffuseTexture == nullptr ? false : true);
	if(m_pDiffuseTexture)
		m_pDiffuseTextureVar->SetResource(m_pDiffuseTexture->GetResourceView());
}

void DefaultMaterial::SetDiffuseTexture(const wstring& filePath)
{
	m_pDiffuseTexture = ResourceManager::Load<Texture>(filePath);
}
