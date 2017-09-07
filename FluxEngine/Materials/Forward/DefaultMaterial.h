#pragma once
#include "../Material.h"

class Texture;

class DefaultMaterial : public Material
{
public:
	DefaultMaterial();
	~DefaultMaterial();

	void LoadShaderVariables() override;
	void UpdateShaderVariables(MeshRenderer* pMeshComponent) override;

	void SetColor(const Vector4 &color) { m_Color = color; }
	void SetDiffuseTexture(const string& filePath);

private:

	static ID3DX11EffectVectorVariable* m_pColorVar;
	static ID3DX11EffectShaderResourceVariable* m_pDiffuseTextureVar;
	static ID3DX11EffectScalarVariable* m_pUseDiffuseTextureVar;
	Vector4 m_Color;

	Texture* m_pDiffuseTexture = nullptr;
};