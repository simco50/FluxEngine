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

	const BoundingOrientedBox& GetBoundingBox() const { return m_BoundingBox; }

	void SetFrustumCulling(const bool enabled) { m_FrustumCulling = enabled; }

protected:
	void Initialize();
	void Update();
	void Render();

	MeshFilter* m_pMeshFilter = nullptr;
	Material* m_pMaterial = nullptr;

	BoundingOrientedBox m_LocalBoundingBox;
	BoundingOrientedBox m_BoundingBox;

	bool m_FrustumCulling = true;

private:
	friend class ShadowMapper;
};

