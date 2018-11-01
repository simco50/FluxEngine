#include "stdafx.h"
#include "FlexRigidbodyMaterial.h"
#include "../../Graphics/Texture.h"
#include "../../Components/CameraComponent.h"
#include "../../Physics/Flex/FlexHelper.h"
#include "../../Physics/Flex/FlexSystem.h"

ID3DX11EffectVectorVariable* FlexRigidbodyMaterial::m_pLightDirectionVar = nullptr;
ID3DX11EffectVectorVariable* FlexRigidbodyMaterial::m_pColorVar = nullptr;
ID3DX11EffectMatrixVariable* FlexRigidbodyMaterial::m_pViewProjVar = nullptr;
ID3DX11EffectMatrixVariable* FlexRigidbodyMaterial::m_pViewInverseVar = nullptr;
ID3DX11EffectVectorVariable* FlexRigidbodyMaterial::m_pRigidRestPoseVar = nullptr;
ID3DX11EffectVectorVariable* FlexRigidbodyMaterial::m_pRigidTranslationVar = nullptr;
ID3DX11EffectVectorVariable* FlexRigidbodyMaterial::m_pRigidRotationVar = nullptr;
ID3DX11EffectScalarVariable* FlexRigidbodyMaterial::m_pUseDiffuseTextureVar = nullptr;
ID3DX11EffectShaderResourceVariable* FlexRigidbodyMaterial::m_pDiffuseTextureVar = nullptr;

FlexRigidbodyMaterial::FlexRigidbodyMaterial(FlexSystem* pFlexSystem, FlexHelper::FlexMeshInstance* pMeshInstance) :
	m_pMeshInstance(pMeshInstance), m_pFlexSystem(pFlexSystem)
{
	m_MaterialDesc.EffectName = L"./Resources/Shaders/Flex/FlexRigidBody.fx";
}


FlexRigidbodyMaterial::~FlexRigidbodyMaterial()
{
}

void FlexRigidbodyMaterial::LoadShaderVariables()
{
	BIND_AND_CHECK_NAME(m_pColorVar, gColor, AsVector);
	m_pColorVar->GetFloatVector((float*)&m_Color);
	BIND_AND_CHECK_NAME(m_pLightDirectionVar, gLightDirection, AsVector);
	BIND_AND_CHECK_NAME(m_pViewProjVar, gViewProj, AsMatrix);
	BIND_AND_CHECK_NAME(m_pViewInverseVar, gViewInv, AsMatrix);
	BIND_AND_CHECK_NAME(m_pDiffuseTextureVar, gTexture, AsShaderResource);
	BIND_AND_CHECK_NAME(m_pUseDiffuseTextureVar, gUseDiffuseTexture, AsScalar);

	BIND_AND_CHECK_NAME(m_pRigidRestPoseVar, gRigidRestpose, AsVector);
	BIND_AND_CHECK_NAME(m_pRigidTranslationVar, gRigidTranslation, AsVector);
	BIND_AND_CHECK_NAME(m_pRigidRotationVar, gRigidRotation, AsVector);
}

void FlexRigidbodyMaterial::UpdateShaderVariables(MeshComponent* pMeshComponent)
{
	UNREFERENCED_PARAMETER(pMeshComponent);
	m_pColorVar->SetFloatVector(reinterpret_cast<const float*>(&m_Color));
	m_pViewProjVar->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetViewProjection()));
	m_pViewInverseVar->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetViewInverse()));

	if (m_pTexture) m_pDiffuseTextureVar->SetResource(m_pTexture->GetResourceView());
	m_pUseDiffuseTextureVar->SetBool(m_pTexture ? true : false);

	m_pRigidRestPoseVar->SetFloatVector((float*)&m_pMeshInstance->RigidRestPoses[0]);
	m_pRigidRotationVar->SetFloatVector((float*)&m_pFlexSystem->RigidRotations[m_pMeshInstance->Offset]);
	m_pRigidTranslationVar->SetFloatVector((float*)&m_pFlexSystem->RigidTranslations[m_pMeshInstance->Offset]);
}