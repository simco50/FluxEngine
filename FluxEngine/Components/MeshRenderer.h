#pragma once
#include "ComponentBase.h"

class MeshFilter;
class Material;

class MeshRenderer : public ComponentBase
{
public:
	MeshRenderer();
	MeshRenderer(MeshFilter* pMeshFilter);
	MeshRenderer(const string& meshPath);
	~MeshRenderer();

	void SetMaterial(Material* pMaterial);

	Material* GetMaterial() const { return m_pMaterial; }

	const BoundingOrientedBox& GetBoundingBox() const { return m_BoundingBox; }

	void SetFrustumCulling(const bool enabled) { m_FrustumCulling = enabled; }

	MeshFilter* GetMeshFilter() { return m_pMeshFilter; }

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

