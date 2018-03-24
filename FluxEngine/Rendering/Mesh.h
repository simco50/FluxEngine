#pragma once
#include "Content/Resource.h"
#include "Skeleton.h"
#include "Animation.h"

class VertexBuffer;
class IndexBuffer;
class Geometry;
class Graphics;
struct VertexElement;

struct aiScene;

class Mesh : public Resource
{
	FLUX_OBJECT(Mesh, Resource)

public:
	Mesh(Context* pContext);
	virtual ~Mesh();

	DELETE_COPY(Mesh)

	virtual bool Load(InputStream& inputStream) override;

	void CreateBuffers(std::vector<VertexElement>& elementDesc);

	int GetGeometryCount() const { return m_GeometryCount; }
	Geometry* GetGeometry(int slot) const { return m_Geometries[slot].get(); }
	const BoundingBox& GetBoundingBox() const { return m_BoundingBox; }

	std::vector<Matrix> GetBoneMatrices() const
	{
		return m_Animations[0].GetBoneMatrices(0);
	}

private:
	bool LoadFlux(InputStream& inputStream);
	bool LoadAssimp(InputStream& inputStream);

	bool ProcessAssimpMeshes(const aiScene* pScene);
	bool ProcessAssimpAnimations(const aiScene* pScene);

	static const int MESH_VERSION = 7;

	void CreateBuffersForGeometry(std::vector<VertexElement>& elementDesc, Geometry* pGeometry);

	std::string m_MeshName;
	bool m_BuffersInitialized = false;
	std::vector<std::unique_ptr<VertexBuffer>> m_VertexBuffers;
	std::vector<std::unique_ptr<IndexBuffer>> m_IndexBuffers;

	std::vector<std::unique_ptr<Geometry>> m_Geometries;
	BoundingBox m_BoundingBox;

	Skeleton m_Skeleton;
	std::vector<Animation> m_Animations;

	int m_GeometryCount = 0;
};