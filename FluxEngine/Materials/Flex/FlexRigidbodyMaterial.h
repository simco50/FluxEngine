#pragma once
#include "..\Material.h"

class Texture;
class FlexSystem;
namespace FlexHelper
{
	struct FlexMeshInstance;
}

class FlexRigidbodyMaterial : public Material
{
public:
	FlexRigidbodyMaterial(FlexSystem* pFlexSystem, FlexHelper::FlexMeshInstance* pMeshInstance);
	~FlexRigidbodyMaterial();
	void SetTexture(Texture* pTexture) { m_pTexture = pTexture; }

protected:
	void LoadShaderVariables();
	void UpdateShaderVariables(MeshComponent* pMeshComponent);

private:
	FlexHelper::FlexMeshInstance* m_pMeshInstance;
	FlexSystem* m_pFlexSystem;

	static ID3DX11EffectVectorVariable* m_pLightDirectionVar;
	static ID3DX11EffectVectorVariable* m_pColorVar;
	static ID3DX11EffectMatrixVariable* m_pViewProjVar;
	static ID3DX11EffectMatrixVariable* m_pViewInverseVar;
	static ID3DX11EffectVectorVariable* m_pRigidRestPoseVar;
	static ID3DX11EffectVectorVariable* m_pRigidTranslationVar;
	static ID3DX11EffectVectorVariable* m_pRigidRotationVar;
	static ID3DX11EffectScalarVariable* m_pMeshOffsetVar;
	static ID3DX11EffectScalarVariable* m_pUseDiffuseTextureVar;
	static ID3DX11EffectShaderResourceVariable* m_pDiffuseTextureVar;

	XMFLOAT4 m_Color;
	Texture* m_pTexture = nullptr;
};

