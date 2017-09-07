#pragma once
#include "../Material.h"

class Texture;

class BasicMaterial_Deferred : public Material
{
public:
	BasicMaterial_Deferred();
	~BasicMaterial_Deferred();

	void LoadShaderVariables() override;
	void UpdateShaderVariables(MeshRenderer* pMeshComponent) override;

	void SetDiffuseTexture(Texture* pTexture);
	void SetNormalTexture(Texture* pTexture);

private:
	ID3DX11EffectScalarVariable* m_pUseDiffuseTexture = nullptr;
	ID3DX11EffectShaderResourceVariable* m_pDiffuseTexture = nullptr; 
	ID3DX11EffectScalarVariable* m_pUseNormalTexture = nullptr;
	ID3DX11EffectShaderResourceVariable* m_pNormalTexture = nullptr;

	bool m_UseDiffuseTexture = false;
	bool m_UseNormalMap = false;
	Texture* m_pDiffuseMap = nullptr;
	Texture* m_pNormalMap = nullptr;
};

