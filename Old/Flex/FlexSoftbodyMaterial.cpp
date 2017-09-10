#include "stdafx.h"
#include "FlexSoftbodyMaterial.h"
#include "../../Physics/Flex/FlexHelper.h"
#include "../../Components/CameraComponent.h"
#include "../../Graphics/Texture.h"
#include "../../Physics/Flex/FlexSystem.h"

ID3DX11EffectVectorVariable* FlexSoftbodyMaterial::m_pLightDirectionVar = nullptr;
ID3DX11EffectVectorVariable* FlexSoftbodyMaterial::m_pColorVar = nullptr;
ID3DX11EffectMatrixVariable* FlexSoftbodyMaterial::m_pViewProjVar = nullptr;
ID3DX11EffectMatrixVariable* FlexSoftbodyMaterial::m_pViewInverseVar = nullptr;
ID3DX11EffectVectorVariable* FlexSoftbodyMaterial::m_pRigidRestPosesVar = nullptr;
ID3DX11EffectVectorVariable* FlexSoftbodyMaterial::m_pRigidTranslationsVar = nullptr;
ID3DX11EffectVectorVariable* FlexSoftbodyMaterial::m_pRigidRotationsVar = nullptr;
ID3DX11EffectScalarVariable* FlexSoftbodyMaterial::m_pUseDiffuseTextureVar = nullptr;
ID3DX11EffectShaderResourceVariable* FlexSoftbodyMaterial::m_pDiffuseTextureVar = nullptr;

FlexSoftbodyMaterial::FlexSoftbodyMaterial(FlexSystem* pFlexSystem, FlexHelper::FlexMeshInstance* pMeshInstance):
m_pMeshInstance(pMeshInstance), m_pFlexSystem(pFlexSystem)
{
	m_MaterialDesc.EffectName = "./Resources/Shaders/Flex/FlexSoftBody.fx";
}


FlexSoftbodyMaterial::~FlexSoftbodyMaterial()
{
}

void FlexSoftbodyMaterial::LoadShaderVariables()
{
	BIND_AND_CHECK_NAME(m_pColorVar, gColor, AsVector);
	m_pColorVar->GetFloatVector((float*)&m_Color);
	BIND_AND_CHECK_NAME(m_pLightDirectionVar, gLightDirection, AsVector);
	BIND_AND_CHECK_NAME(m_pViewProjVar, gViewProj, AsMatrix);
	BIND_AND_CHECK_NAME(m_pViewInverseVar, gViewInv, AsMatrix);

	BIND_AND_CHECK_NAME(m_pRigidRestPosesVar, gRigidRestposes, AsVector);
	BIND_AND_CHECK_NAME(m_pRigidTranslationsVar, gRigidTranslations, AsVector);
	BIND_AND_CHECK_NAME(m_pRigidRotationsVar, gRigidRotations, AsVector);
	BIND_AND_CHECK_NAME(m_pDiffuseTextureVar, gTexture, AsShaderResource);
	BIND_AND_CHECK_NAME(m_pUseDiffuseTextureVar, gUseDiffuseTexture, AsScalar);
}

void FlexSoftbodyMaterial::UpdateShaderVariables(MeshComponent* pMeshComponent)
{
	UNREFERENCED_PARAMETER(pMeshComponent);

	m_pColorVar->SetFloatVector(reinterpret_cast<const float*>(&m_Color));
	m_pViewProjVar->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetViewProjection()));
	m_pViewInverseVar->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetViewInverse()));

	m_pRigidRestPosesVar->SetFloatVectorArray((float*)m_pMeshInstance->RigidRestPoses.data(), 0, m_pMeshInstance->RigidRestPoses.size());
	m_pRigidRotationsVar->SetFloatVectorArray((float*)&m_pFlexSystem->RigidRotations[m_pMeshInstance->Offset], 0, m_pMeshInstance->RigidRestPoses.size());
	m_pRigidTranslationsVar->SetFloatVectorArray((float*)&m_pFlexSystem->RigidTranslations[m_pMeshInstance->Offset], 0, m_pMeshInstance->RigidRestPoses.size());
	if (m_pTexture) m_pDiffuseTextureVar->SetResource(m_pTexture->GetResourceView());
	m_pUseDiffuseTextureVar->SetBool(m_pTexture ? true : false);
}

void FlexSoftbodyMaterial::CreateInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC desc[] =
	{
		//Per meshfilter
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA,0 },
		//Per softbody
		{ "WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "CLUSTER", 0, DXGI_FORMAT_R32G32B32A32_SINT, 1, 16, D3D11_INPUT_PER_VERTEX_DATA,0 },
	};
	UINT numElements = sizeof(desc) / sizeof(desc[0]);
	D3DX11_PASS_DESC passDesc;
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(m_pGameContext->Engine->D3Device->CreateInputLayout(desc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, m_pInputLayout.GetAddressOf() ))
}
