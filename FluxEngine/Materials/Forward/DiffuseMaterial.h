#pragma once
#include "../Material.h"

class Texture;

class DiffuseMaterial : public Material
{
public:
	DiffuseMaterial();
	~DiffuseMaterial();

	void SetTexture(Texture* pTexture);

private:
	void LoadShaderVariables() override;
	void UpdateShaderVariables(MeshComponent* pMeshComponent) override;

	ID3DX11EffectShaderResourceVariable* m_pDiffuseTextureVariable = nullptr;
};

