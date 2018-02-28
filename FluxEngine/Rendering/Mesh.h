#pragma once
#include "Content/Resource.h"

class VertexBuffer;
class IndexBuffer;
class Geometry;
class Graphics;
struct VertexElement;

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

private:
	static const int MESH_VERSION = 7;

	void CreateBuffersForGeometry(std::vector<VertexElement>& elementDesc, Geometry* pGeometry);

	std::string m_MeshName;
	bool m_BuffersInitialized = false;
	std::vector<std::unique_ptr<VertexBuffer>> m_VertexBuffers;
	std::vector<std::unique_ptr<IndexBuffer>> m_IndexBuffers;

	std::vector<std::unique_ptr<Geometry>> m_Geometries;
	BoundingBox m_BoundingBox;

	int m_GeometryCount = 0;
};