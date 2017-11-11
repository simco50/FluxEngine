#pragma once
#include "Content/Resource.h"

class VertexBuffer;
class IndexBuffer;
class Geometry;
struct VertexElement;

class Mesh : public Resource
{
public:
	Mesh();
	~Mesh();

	DELETE_COPY(Mesh)

	virtual bool Load(const std::string& filePath) override;

	void CreateBuffers(Graphics* pGraphics, vector<VertexElement>& elementDesc);

	int GetGeometryCount() const { return m_GeometryCount; }
	Geometry* GetGeometry(int slot) const { return m_Geometries[slot].get(); }

private:
	static const int MESH_VERSION = 6;

	void CreateBuffersForGeometry(Graphics* pGraphics, vector<VertexElement>& elementDesc, Geometry* pGeometry);

	string m_MeshName;
	bool m_BuffersInitialized = false;
	vector<unique_ptr<VertexBuffer>> m_VertexBuffers;
	vector<unique_ptr<IndexBuffer>> m_IndexBuffers;

	vector<unique_ptr<Geometry>> m_Geometries;

	int m_GeometryCount = 0;
};

