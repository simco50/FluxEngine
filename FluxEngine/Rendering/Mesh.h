#pragma once
#include "Content/Resource.h"
#include "Animation/Skeleton.h"
#include "Animation/Animation.h"

class VertexBuffer;
class IndexBuffer;
class Geometry;
class Graphics;
class Texture;
struct VertexElement;

struct aiScene;
struct aiNode;

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

	const Skeleton& GetSkeleton() const { return m_Skeleton; }

	Animation* GetAnimation(const std::string& name) const;
	Animation* GetAnimation(const StringHash hash) const;
	Animation* GetAnimation(const int index) const;

private:
	bool LoadFlux(InputStream& inputStream);
	bool LoadAssimp(InputStream& inputStream);

	bool ProcessAssimpMeshes(const aiScene* pScene);
	bool ProcessAssimpAnimations(const aiScene* pScene);
	bool ProcessSkeleton(const aiScene* pScene);
	void CalculateBoundingBox();
	void ProcessNode(aiNode* pNode, Matrix parentMatrix, Bone* pParentBone = nullptr);

	static const int MESH_VERSION = 7;

	void CreateBuffersForGeometry(std::vector<VertexElement>& elementDesc, Geometry* pGeometry);
	void RefreshMemoryUsage();

	std::string m_MeshName;
	std::vector<std::unique_ptr<VertexBuffer>> m_VertexBuffers;
	std::vector<std::unique_ptr<IndexBuffer>> m_IndexBuffers;

	std::vector<std::unique_ptr<Geometry>> m_Geometries;
	BoundingBox m_BoundingBox;

	std::map<std::string, int> m_BoneMap;
	Skeleton m_Skeleton;
	std::vector<std::unique_ptr<Animation>> m_Animations;
	int m_GeometryCount = 0;
};