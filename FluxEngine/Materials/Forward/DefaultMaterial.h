#pragma once
#include "../Material.h"

class DefaultMaterial : public Material
{
public:
	DefaultMaterial();
	~DefaultMaterial();

	void LoadShaderVariables() override;
	void UpdateShaderVariables(MeshComponent* pMeshComponent) override;

	void SetColor(const Vector4 &color) { m_Color = color; }

private:

	ID3DX11EffectVectorVariable* m_pColorVar = nullptr;

	Vector4 m_Color;
};