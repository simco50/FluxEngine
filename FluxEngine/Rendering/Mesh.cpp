#include "FluxEngine.h"
#include "Mesh.h"

#include "Rendering/Core/IndexBuffer.h"
#include "Rendering/Core/VertexBuffer.h"
#include "Geometry.h"

#include "Async/AsyncTaskQueue.h"
#include "Core/Graphics.h"
#include "IO/InputStream.h"
#include "Content/AssimpHelpers.h"

Mesh::Mesh(Context* pContext):
	Resource(pContext)
{
}

Mesh::~Mesh()
{
}

bool Mesh::Load(InputStream& inputStream)
{
	std::string fileName = inputStream.GetSource();
	AUTOPROFILE_DESC(Mesh_Load, Paths::GetFileName(fileName));

	m_Geometries.clear();
	m_MeshName = Paths::GetFileName(fileName);

	std::string extension = Paths::GetFileExtenstion(fileName);
	if (extension == "flux")
	{
		if (!LoadFlux(inputStream))
		{
			return false;
		}
	}
	else
	{
		FLUX_LOG(Warning, "[Mesh::Load] Slow loading '%s' using Assimp", fileName.c_str());
		if (!LoadAssimp(inputStream))
		{
			return false;
		}
	}

	RefreshMemoryUsage();

	return true;
}

void Mesh::CreateBuffers(std::vector<VertexElement>& elementDesc)
{
	AUTOPROFILE_DESC(Mesh_CreateBuffers, m_MeshName);
	for (std::unique_ptr<Geometry>& pGeometry : m_Geometries)
	{
		CreateBuffersForGeometry(elementDesc, pGeometry.get());
	}
}

bool Mesh::LoadFlux(InputStream& inputStream)
{
	AUTOPROFILE(Mesh_Load_Flux);

	const std::string magic = inputStream.ReadSizedString();
	const char minVersion = inputStream.ReadByte();
	/*const char maxVersion =*/ inputStream.ReadByte();

	if (minVersion != MESH_VERSION)
	{
		FLUX_LOG(Warning, "[MeshLoader::LoadContent()] File version mismatch: Expects v%d.0 but is v%d.0", MESH_VERSION, minVersion);
		return false;
	}

	inputStream.Read((char*)&m_BoundingBox, sizeof(BoundingBox));

	m_GeometryCount = inputStream.ReadInt();

	for (int i = 0; i < m_GeometryCount; ++i)
	{
		std::unique_ptr<Geometry> pGeometry = std::make_unique<Geometry>();
		for (;;)
		{
			std::string block = inputStream.ReadSizedString();
			for (char& c : block)
			{
				c = (char)toupper(c);
			}
			if (block == "ENDMESH")
			{
				break;
			}

			const unsigned int length = inputStream.ReadUInt();
			const unsigned int stride = inputStream.ReadUInt();

			pGeometry->GetVertexDataUnsafe(block).pData = new char[length * stride];
			pGeometry->GetVertexDataUnsafe(block).Count = length;
			pGeometry->GetVertexDataUnsafe(block).Stride = stride;
			inputStream.Read((char*)pGeometry->GetVertexDataUnsafe(block).pData, length * stride);
		}

		pGeometry->SetDrawRange(PrimitiveType::TRIANGLELIST, pGeometry->GetDataCount("INDEX"), pGeometry->GetDataCount("POSITION"));
		m_Geometries.push_back(std::move(pGeometry));
	}
	return true;
}

bool Mesh::LoadAssimp(InputStream& inputStream)
{
	AUTOPROFILE(Mesh_Load_Assimp);

	const aiScene* pScene = nullptr;
	Assimp::Importer importer;
	{
		AUTOPROFILE(Mesh_ImportAssimp);
		pScene = AssimpHelpers::LoadScene(inputStream, importer);
	}

	if (pScene == nullptr)
	{
		return false;
	}
	if (!ProcessAssimpMeshes(pScene))
	{
		return false;
	}
	{
		AUTOPROFILE(Mesh_ProcessSkeletonHierarchy);
		ProcessSkeletonHierarchy(pScene->mRootNode, nullptr);
	}
	if (m_Geometries.size() > 0)
	{
		CalculateBoundingBox();
	}
	return true;
}

bool Mesh::ProcessAssimpMeshes(const aiScene* pScene)
{
	AUTOPROFILE(Mesh_Load_Meshes);

	m_GeometryCount = pScene->mNumMeshes;
	int boneCount = 0;
	for (unsigned int i = 0; i < (unsigned int)m_GeometryCount; i++)
	{
		aiMesh* pMesh = pScene->mMeshes[i];
		std::unique_ptr<Geometry> pGeometry = std::make_unique<Geometry>();
		pGeometry->SetDrawRange(PrimitiveType::TRIANGLELIST, pMesh->mNumFaces * 3, pMesh->mNumVertices);

		if (pMesh->HasPositions())
		{
			Geometry::VertexData& data = pGeometry->GetVertexDataUnsafe("POSITION");
			data.Count = pMesh->mNumVertices;
			data.Stride = sizeof(aiVector3D);
			data.CreateBuffer();
			memcpy(data.pData, &pMesh->mVertices[0].x, data.ByteSize());
		}
		if (pMesh->HasNormals())
		{
			Geometry::VertexData& data = pGeometry->GetVertexDataUnsafe("NORMAL");
			data.Count = pMesh->mNumVertices;
			data.Stride = sizeof(aiVector3D);
			data.CreateBuffer();
			memcpy(data.pData, &pMesh->mNormals[0].x, data.ByteSize());
		}
		if (pMesh->HasTangentsAndBitangents())
		{
			Geometry::VertexData& data = pGeometry->GetVertexDataUnsafe("TANGENT");
			data.Count = pMesh->mNumVertices;
			data.Stride = sizeof(aiVector3D);
			data.CreateBuffer();
			memcpy(data.pData, &pMesh->mTangents[0].x, data.ByteSize());
		}

		for (int j = 0; j < GraphicsConstants::MAX_UV_CHANNELS; ++j)
		{
			if (pMesh->HasTextureCoords(j))
			{
				Geometry::VertexData& data = pGeometry->GetVertexDataUnsafe("TEXCOORD", j);
				data.Count = pMesh->mNumVertices;
				data.Stride = sizeof(aiVector2D);
				data.CreateBuffer();
				Vector2* pCurrent = reinterpret_cast<Vector2*>(data.pData);
				for (int k = 0; k < data.Count; k++)
				{
					pCurrent[k] = Vector2{ pMesh->mTextureCoords[j][k].x, pMesh->mTextureCoords[j][k].y };
				}
			}
		}

		if (pMesh->HasFaces())
		{
			Geometry::VertexData& data = pGeometry->GetVertexDataUnsafe("INDEX");
			data.Count = pMesh->mNumFaces * 3;
			data.Stride = sizeof(unsigned int);
			data.CreateBuffer();
			unsigned int* pCurrent = reinterpret_cast<unsigned int*>(data.pData);
			for (unsigned int j = 0; j < pMesh->mNumFaces; j++)
			{
				memcpy(pCurrent + j * 3, pMesh->mFaces[j].mIndices, sizeof(unsigned int) * 3);
			}
		}

		if (pMesh->HasBones())
		{
			Geometry::VertexData& indexData = pGeometry->GetVertexDataUnsafe("BLENDINDEX");
			indexData.Count = pMesh->mNumVertices;
			indexData.Stride = sizeof(unsigned int) * GraphicsConstants::MAX_BONES_PER_VERTEX;
			indexData.CreateBuffer();
			memset(indexData.pData, -1, indexData.ByteSize());

			Geometry::VertexData& weightData = pGeometry->GetVertexDataUnsafe("BLENDWEIGHT");
			weightData.Count = pMesh->mNumVertices;
			weightData.Stride = sizeof(float) * GraphicsConstants::MAX_BONES_PER_VERTEX;
			weightData.CreateBuffer();
			memset(weightData.pData, 0, indexData.ByteSize());

			float* pWeights = reinterpret_cast<float*>(weightData.pData);
			int* pIndices = reinterpret_cast<int*>(indexData.pData);

			for (unsigned int boneIndex = 0; boneIndex < pMesh->mNumBones; boneIndex++)
			{
				aiBone* pBone = pMesh->mBones[boneIndex];
				if (m_BoneMap.find(pBone->mName.C_Str()) == m_BoneMap.end())
				{
					m_BoneMap[pBone->mName.C_Str()] = boneCount;
					Bone newBone;
					newBone.Name = pBone->mName.C_Str();
					newBone.OffsetMatrix = AssimpHelpers::ToDXMatrix(pBone->mOffsetMatrix);
					//Since we just keep pushing back at the end of the bones, we can safetely assume the bone index is the same as the index in the bone array
					m_Skeleton.AddBone(newBone);
					++boneCount;
				}
				int index = m_BoneMap[pBone->mName.C_Str()];

				for (unsigned int weightIndex = 0; weightIndex < pBone->mNumWeights; weightIndex++)
				{
					const aiVertexWeight& pWeight = pBone->mWeights[weightIndex];
					for (int j = 0; j < 4; j++)
					{
						if (pWeights[pWeight.mVertexId * GraphicsConstants::MAX_BONES_PER_VERTEX + j] == 0.0f)
						{
							pWeights[pWeight.mVertexId * GraphicsConstants::MAX_BONES_PER_VERTEX + j] = pWeight.mWeight;
							pIndices[pWeight.mVertexId * GraphicsConstants::MAX_BONES_PER_VERTEX + j] = index;
							break;
						}
					}
				}
			}
		}
		m_Geometries.push_back(std::move(pGeometry));
	}
	return true;
}

void Mesh::CalculateBoundingBox()
{
	AUTOPROFILE(Mesh_CalculateBoundingBox);

	for (auto& pGeometry : m_Geometries)
	{
		BoundingBox bb;
		BoundingBox::CreateFromPoints(bb, pGeometry->GetVertexCount(), (const Vector3*)pGeometry->GetVertexData("POSITION").pData, (int)sizeof(Vector3));
		m_BoundingBox.CreateMerged(m_BoundingBox, m_BoundingBox, bb);
	}
}

void Mesh::ProcessSkeletonHierarchy(aiNode* pNode, Bone* pParentBone)
{
	Bone* pBone = m_Skeleton.GetBone(pNode->mName.C_Str());

	auto pIt = m_BoneMap.find(std::string(pNode->mName.C_Str()));
	if (pIt != m_BoneMap.end())
	{
		if (m_Skeleton.GetRootBoneIndex() == -1)
		{
			m_Skeleton.SetParentBoneIndex(pIt->second);
		}
		else
		{
			pParentBone->Children.push_back(pIt->second);
		}
		pParentBone = pBone;

		aiQuaternion rotation;
		aiVector3D scale, translation;
		pNode->mTransformation.Decompose(scale, rotation, translation);
		pBone->StartPosition = AssimpHelpers::ToDXVector3(translation);
		pBone->StartScale = AssimpHelpers::ToDXVector3(scale);
		pBone->StartRotation = AssimpHelpers::TxDXQuaternion(rotation);
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; ++i)
	{
		ProcessSkeletonHierarchy(pNode->mChildren[i], pParentBone);
	}
}

void Mesh::CreateBuffersForGeometry(std::vector<VertexElement>& elementDesc, Geometry* pGeometry)
{
	AUTOPROFILE(Mesh_CreateGeometryBuffers);

	std::unique_ptr<VertexBuffer> pVertexBuffer = std::make_unique<VertexBuffer>(GetSubsystem<Graphics>());
	pVertexBuffer->Create(pGeometry->GetVertexCount(), elementDesc, false);

	int vertexStride = pVertexBuffer->GetVertexStride();
	if (vertexStride == 0)
	{
		FLUX_LOG(Error, "[MeshFilter::CreateBuffers()] VertexStride of the InputLayout is 0");
		return;
	}

	bool hasElements = true;
	for (VertexElement& element : elementDesc)
	{
		const char* pSemantic = VertexElement::GetSemanticOfType(element.Semantic);
		if (pGeometry->HasData(pSemantic) == false)
		{
			FLUX_LOG(Warning, "[MeshFilter::CreateBuffers()] Geometry has no %s", pSemantic);
			hasElements = false;
		}
	}
	if (hasElements == false)
	{
		return;
	}

	char* pDataLocation = new char[vertexStride * pGeometry->GetVertexCount()];
	char* pVertexDataStart = pDataLocation;

	AsyncTaskQueue* pQueue = GetSubsystem<AsyncTaskQueue>();
	for (size_t i = 0; i < elementDesc.size(); ++i)
	{
		const Geometry::VertexData* pData = &pGeometry->GetVertexData(VertexElement::GetSemanticOfType(elementDesc[i].Semantic));
		checkf(pData, "[Mesh::CreateBufferForGeometry] Mesh does not have the appropriate data of semantic");

		int elementSize = elementDesc[i].GetSizeOfType(elementDesc[i].Type);
		AsyncTaskDelegate taskFunction = AsyncTaskDelegate::CreateLambda([pGeometry, pData, pDataLocation, vertexStride, elementSize](AsyncTask*, int)
		{
			char* pStart = pDataLocation;
			for (int j = 0; j < pGeometry->GetVertexCount(); ++j)
			{
				memcpy(pStart, (char*)pData->pData + j * elementSize, elementSize);
				pStart += vertexStride;
			}
		}
		);
		pQueue->AddWorkItem(taskFunction);

		pDataLocation += elementSize;
	}

	if (pGeometry->HasData("INDEX"))
	{
		AUTOPROFILE(Mesh_CreateIndexBufferForGeometry);

		std::unique_ptr<IndexBuffer> pIndexBuffer = std::make_unique<IndexBuffer>(GetSubsystem<Graphics>());
		pIndexBuffer->Create(pGeometry->GetIndexCount(), false, false);
		pIndexBuffer->SetData(pGeometry->GetVertexData("INDEX").pData);
		pGeometry->SetIndexBuffer(pIndexBuffer.get());
		m_IndexBuffers.push_back(std::move(pIndexBuffer));
	}

	pQueue->JoinAll();

	pVertexBuffer->SetData(pVertexDataStart);
	pGeometry->SetVertexBuffer(pVertexBuffer.get());
	m_VertexBuffers.push_back(std::move(pVertexBuffer));

	delete[] pVertexDataStart;
}

void Mesh::RefreshMemoryUsage()
{
	unsigned int memoryUsage = 0;
	for(const auto& pGeometry : m_Geometries)
	{
		memoryUsage += pGeometry->GetSize();
	}
	memoryUsage += (unsigned int)m_Skeleton.BoneCount() * sizeof(Bone);

	SetMemoryUsage(memoryUsage);
}