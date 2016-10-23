#pragma once
#include "../Materials/Material.h"

class ShadowMapMaterial : public Material
{
public:
	ShadowMapMaterial();
	~ShadowMapMaterial();

	void SetLightVP(const XMFLOAT4X4& matrix);

	void LoadShaderVariables();
	void UpdateShaderVariables(MeshComponent* pMeshComponent);

private:
	ID3DX11EffectMatrixVariable* m_pLightVPV = nullptr;
};

