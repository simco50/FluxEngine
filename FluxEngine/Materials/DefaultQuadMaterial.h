#pragma once
#include "Material.h"

class Texture;

class DefaultQuadMaterial :	public Material
{
public:
	DefaultQuadMaterial();
	~DefaultQuadMaterial();

	void SetTexture(ID3D11ShaderResourceView* pSRV);

protected:
	void LoadShaderVariables(){}
	void UpdateShaderVariables(MeshRenderer* pMeshComponent) { UNREFERENCED_PARAMETER(pMeshComponent); }
};

