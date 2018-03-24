#pragma once
#include "Content/Resource.h"

class VertexBuffer;
class IndexBuffer;
class Geometry;
class Graphics;
struct VertexElement;

struct aiScene;

struct Bone
{
	int Index;
	std::string Name;
	Matrix OffsetMatrix;
};

struct Skeleton
{
	std::vector<Bone> Bones;
};

struct AnimationKey
{
	Vector3 Position;
	Quaternion Rotation;
	Vector3 Scale;
};

struct AnimationNode
{
	int BoneIndex;
	std::string Name;
	std::vector<std::pair<float, AnimationKey>> Keys;
};

struct Animation
{
	std::string Name;
	float Duration;
	float TicksPerSecond;
	std::vector<AnimationNode> Nodes;
};

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
	bool LoadFlux(InputStream& inputStream);
	bool LoadAssimp(InputStream& inputStream);

	bool ProcessAssimpMeshes(const aiScene* pScene);
	bool ProcessAssimpAnimations(const aiScene* pScene);

	void AddWeight(int index, int boneIndex, float weight)
	{
		for (size_t i = 0; i < m_BoneWeights[index].size(); ++i)
		{
			if (m_BoneWeights[index][i] == 0.0f)
			{
				m_BoneWeights[index][i] = weight;
				m_BoneIndices[index][i] = boneIndex;
				return;
			}
		}
	}

	static const int MESH_VERSION = 7;

	void CreateBuffersForGeometry(std::vector<VertexElement>& elementDesc, Geometry* pGeometry);

	std::string m_MeshName;
	bool m_BuffersInitialized = false;
	std::vector<std::unique_ptr<VertexBuffer>> m_VertexBuffers;
	std::vector<std::unique_ptr<IndexBuffer>> m_IndexBuffers;

	std::vector<std::array<int, 4>> m_BoneIndices;
	std::vector<std::array<float, 4>> m_BoneWeights;

	std::vector<std::unique_ptr<Geometry>> m_Geometries;
	BoundingBox m_BoundingBox;

	Skeleton m_Skeleton;
	std::vector<Animation> m_Animations;

	int m_GeometryCount = 0;
};