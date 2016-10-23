#pragma once
#include "ComponentBase.h"

class MeshFilter;
class Material;

class MeshComponent : public ComponentBase
{
public:
	MeshComponent();
	MeshComponent(MeshFilter* pMeshFilter);
	MeshComponent(const wstring& meshPath);
	~MeshComponent();

	void SetMaterial(Material* pMaterial);

	Material* GetMaterial() const { return m_pMaterial; }

protected:
	void Initialize();
	void Update();
	void Render();

	std::wstring m_EffectFilePath = L"./Resources/HLSL/PosNormCol3D.fx";

	MeshFilter* m_pMeshFilter = nullptr;

	Material* m_pMaterial = nullptr;

private:
	friend class ShadowMapper;
};

