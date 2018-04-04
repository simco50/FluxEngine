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

	std::vector<Matrix> GetBoneMatrices()
	{
		return m_Animations[0].GetBoneMatrices(GameTimer::GameTime(), m_Skeleton);
	}

	const Skeleton& GetSkeleton() const { return m_Skeleton; }

private:
	bool LoadFlux(InputStream& inputStream);
	bool LoadAssimp(InputStream& inputStream);

	bool ProcessAssimpMeshes(const aiScene* pScene);
	bool ProcessAssimpAnimations(const aiScene* pScene);
	bool ProcessSkeleton(const aiScene* pScene);
	void ProcessNode(aiNode* pNode, Matrix parentMatrix, Bone* pParentBone = nullptr);

	static const int MESH_VERSION = 7;

	void CreateBuffersForGeometry(std::vector<VertexElement>& elementDesc, Geometry* pGeometry);

	std::string m_MeshName;
	bool m_BuffersInitialized = false;
	std::vector<std::unique_ptr<VertexBuffer>> m_VertexBuffers;
	std::vector<std::unique_ptr<IndexBuffer>> m_IndexBuffers;

	std::vector<std::unique_ptr<Geometry>> m_Geometries;
	BoundingBox m_BoundingBox;

	std::map<std::string, int> m_BoneMap;
	Skeleton m_Skeleton;
	Matrix m_GlobalTransform;
	Matrix m_InverseGlobalTransform;
	std::vector<Animation> m_Animations;

	int m_GeometryCount = 0;
};