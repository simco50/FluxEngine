#pragma once
#include "Material.h"

class Texture;

class SkyboxMaterial : public Material
{
public:
	SkyboxMaterial();
	~SkyboxMaterial();

	void LoadShaderVariables();
	void UpdateShaderVariables();

	void SetTexture(Texture* pTexture) { m_pTexture = pTexture; }

private:
	Texture* m_pTexture = nullptr;
	ID3DX11EffectShaderResourceVariable* m_pTextureVar = nullptr;
};

