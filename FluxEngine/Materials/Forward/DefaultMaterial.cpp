#include "stdafx.h"
#include "DefaultMaterial.h"


DefaultMaterial::DefaultMaterial() : Material()
{
	m_MaterialDesc.EffectName = L"./Resources/Shaders/Default_Forward.fx";
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
}

void DefaultMaterial::UpdateShaderVariables(MeshComponent* pMeshComponent)
{
	UNREFERENCED_PARAMETER(pMeshComponent);
	m_pColorVar->SetFloatVector(&m_Color.x);
}
