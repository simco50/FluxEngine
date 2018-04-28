#include "FluxEngine.h"
#include "Mesh.h"

#include "Rendering/Core/IndexBuffer.h"
#include "Rendering/Core/VertexBuffer.h"
#include "Geometry.h"

#include "Async/AsyncTaskQueue.h"
#include "Core/Graphics.h"
#include "IO/InputStream.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#pragma comment(lib, "assimp-vc140-mt.lib")

Vector3 ToDXVector3(const aiVector3D& vec)
{
	return *reinterpret_cast<const Vector3*>(&vec);
}

Quaternion TxDXQuaternion(const aiQuaternion& quat)
{
	Quaternion out;
	out.x = quat.x;
	out.y = quat.y;
	out.z = quat.z;
	out.w = quat.w;
	return out;
}

Matrix ToDXMatrix(const aiMatrix4x4& mat)
{
	Matrix m;
	m._11 = mat.a1;
	m._21 = mat.a2;
	m._31 = mat.a3;
	m._41 = mat.a4;

	m._12 = mat.b1;
	m._22 = mat.b2;
	m._32 = mat.b3;
	m._42 = mat.b4;

	m._13 = mat.c1;
	m._23 = mat.c2;
	m._33 = mat.c3;
	m._43 = mat.c4;

	m._14 = mat.d1;
	m._24 = mat.d2;
	m._34 = mat.d3;
	m._44 = mat.d4;

	return m;
}


aiQuaternion GetRotation(const aiNodeAnim* pNode, const float time)
{
	if (pNode->mNumRotationKeys == 0)
		return aiQuaternion();
	for (unsigned int i = 0; i < pNode->mNumRotationKeys; i++)
	{
		if (pNode->mRotationKeys[i].mTime == time)
			return pNode->mRotationKeys[i].mValue;
		if (pNode->mRotationKeys[i].mTime > time)
		{
			float t = Math::InverseLerp((float)pNode->mRotationKeys[i - 1].mTime, (float)pNode->mRotationKeys[i].mTime, time);
			aiQuaternion output;
			aiQuaterniont<float>::Interpolate(output, pNode->mRotationKeys[i - 1].mValue, pNode->mRotationKeys[i].mValue, t);
			return output;
		}
	}
	return pNode->mRotationKeys[pNode->mNumRotationKeys - 1].mValue;
}

aiVector3D GetScale(const aiNodeAnim* pNode, const float time)
{
	if (pNode->mNumScalingKeys == 0)
		return aiVector3D();
	for (unsigned int i = 0; i < pNode->mNumScalingKeys; i++)
	{
		if (pNode->mScalingKeys[i].mTime == time)
			return pNode->mScalingKeys[i].mValue;
		if (pNode->mScalingKeys[i].mTime > time)
		{
			float t = Math::InverseLerp((float)pNode->mScalingKeys[i - 1].mTime, (float)pNode->mScalingKeys[i].mTime, time);
			return pNode->mScalingKeys[i - 1].mValue + (pNode->mScalingKeys[i].mValue - pNode->mScalingKeys[i - 1].mValue) * t;
		}
	}
	return pNode->mScalingKeys[pNode->mNumScalingKeys - 1].mValue;
}

aiVector3D GetPosition(const aiNodeAnim* pNode, const float time)
{
	if (pNode->mNumPositionKeys == 0)
		return aiVector3D();
	for (unsigned int i = 0; i < pNode->mNumPositionKeys; i++)
	{
		if (pNode->mPositionKeys[i].mTime == time)
			return pNode->mPositionKeys[i].mValue;
		if (pNode->mPositionKeys[i].mTime > time)
		{
			float t = Math::InverseLerp((float)pNode->mPositionKeys[i - 1].mTime, (float)pNode->mPositionKeys[i].mTime, time);
			return pNode->mPositionKeys[i - 1].mValue + (pNode->mPositionKeys[i].mValue - pNode->mPositionKeys[i - 1].mValue) * t;
		}
	}
	return pNode->mPositionKeys[pNode->mNumPositionKeys - 1].mValue;
}

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

Animation* Mesh::GetAnimation(const std::string name) const
{
	StringHash hash = std::hash<std::string>{}(name);
	return GetAnimation(hash);
}

Animation* Mesh::GetAnimation(const int index) const
{
	assert(index < (int)m_Animations.size());
	return m_Animations[index].get();
}

Animation* Mesh::GetAnimation(const StringHash hash) const
{
	for (const std::unique_ptr<Animation>& pAnimation : m_Animations)
	{
		if(pAnimation->GetNameHash() == hash)
			return pAnimation.get();
	}
	return nullptr;
}

bool Mesh::LoadFlux(InputStream& inputStream)
{
	AUTOPROFILE(Mesh_Load_Flux);

	const std::string magic = inputStream.ReadSizedString();
	const char minVersion = inputStream.ReadByte();
	const char maxVersion = inputStream.ReadByte();

	UNREFERENCED_PARAMETER(maxVersion);
	if (minVersion != MESH_VERSION)
	{
		std::stringstream stream;
		stream << "MeshLoader::LoadContent() File version mismatch: Expects v" << MESH_VERSION << ".0 but is v" << (int)minVersion << ".0";
		FLUX_LOG(Warning, stream.str());
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
				c = (char)toupper(c);
			if (block == "ENDMESH")
				break;

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

	std::vector<unsigned char> buffer;
	inputStream.ReadAllBytes(buffer);

	Assimp::Importer importer;
	const aiScene* pScene;

	{
		AUTOPROFILE(Mesh_ImportAssimp);
		pScene = importer.ReadFileFromMemory(buffer.data(), buffer.size(),
			aiProcess_Triangulate |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace
		);
	}

	if (pScene == nullptr)
		return false;
	if (!ProcessAssimpMeshes(pScene))
		return false;
	std::map<std::string, int> boneMap;
	if (!ProcessSkeleton(pScene))
		return false;
	if (!ProcessAssimpAnimations(pScene))
		return false;
	if(m_Geometries.size() > 0)
		CalculateBoundingBox();
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

		if(pMesh->HasPositions())
		{
			Geometry::VertexData& data = pGeometry->GetVertexDataUnsafe("POSITION");
			data.Count = pMesh->mNumVertices;
			data.Stride = sizeof(aiVector3D);
			data.CreateBuffer();
			memcpy(data.pData, pMesh->mVertices, data.ByteSize());
		}
		if(pMesh->HasNormals())
		{
			Geometry::VertexData& data = pGeometry->GetVertexDataUnsafe("NORMAL");
			data.Count = pMesh->mNumVertices;
			data.Stride = sizeof(aiVector3D);
			data.CreateBuffer();
			memcpy(data.pData, pMesh->mNormals, data.ByteSize());
		}
		if (pMesh->HasTangentsAndBitangents())
		{
			Geometry::VertexData& data = pGeometry->GetVertexDataUnsafe("TANGENT");
			data.Count = pMesh->mNumVertices;
			data.Stride = sizeof(aiVector3D);
			data.CreateBuffer();
			memcpy(data.pData, pMesh->mTangents, data.ByteSize());
		}
		if(pMesh->HasTextureCoords(0))
		{
			Geometry::VertexData& data = pGeometry->GetVertexDataUnsafe("TEXCOORD");
			data.Count = pMesh->mNumVertices;
			data.Stride = sizeof(aiVector2D);
			data.CreateBuffer();
			Vector2* pCurrent = reinterpret_cast<Vector2*>(data.pData);
			for (int j = 0; j < data.Count; j++)
				pCurrent[j] = Vector2{ pMesh->mTextureCoords[0][j].x, pMesh->mTextureCoords[0][j].y };
		}
		if (pMesh->HasFaces())
		{
			Geometry::VertexData& data = pGeometry->GetVertexDataUnsafe("INDEX");
			data.Count = pMesh->mNumFaces * 3;
			data.Stride = sizeof(unsigned int);
			data.CreateBuffer();
			unsigned int* pCurrent = reinterpret_cast<unsigned int*>(data.pData);
			for (unsigned int j = 0; j < pMesh->mNumFaces; j++)
				memcpy(pCurrent + j * 3, pMesh->mFaces[j].mIndices, sizeof(unsigned int) * 3);
		}

		if (pMesh->HasBones())
		{
			Geometry::VertexData& indexData = pGeometry->GetVertexDataUnsafe("BLENDINDEX");
			indexData.Count = pMesh->mNumVertices;
			indexData.Stride = sizeof(unsigned int) * 4;
			indexData.CreateBuffer();
			memset(indexData.pData, -1, indexData.ByteSize());
			
			Geometry::VertexData& weightData = pGeometry->GetVertexDataUnsafe("BLENDWEIGHT");
			weightData.Count = pMesh->mNumVertices;
			weightData.Stride = sizeof(float) * 4;
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
					newBone.Index = boneCount;
					newBone.Name = pBone->mName.C_Str();
					newBone.OffsetMatrix = ToDXMatrix(pBone->mOffsetMatrix);
					m_Skeleton.AddBone(newBone);
					++boneCount;
				}
				int index = m_BoneMap[pBone->mName.C_Str()];

				for (unsigned int weightIndex = 0; weightIndex < pBone->mNumWeights; weightIndex++)
				{
					const aiVertexWeight& pWeight = pBone->mWeights[weightIndex];
					for (int j = 0; j < 4; j++)
					{
						if (pWeights[pWeight.mVertexId * 4 + j] == 0.0f)
						{
							pWeights[pWeight.mVertexId * 4 + j] = pWeight.mWeight;
							pIndices[pWeight.mVertexId * 4 + j] = index;
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

bool Mesh::ProcessAssimpAnimations(const aiScene* pScene)
{
	AUTOPROFILE(Mesh_Load_Animations);

	if (pScene->HasAnimations())
	{
		for (unsigned int i = 0; i < pScene->mNumAnimations; i++)
		{
			const aiAnimation* pAnimation = pScene->mAnimations[i];
			std::unique_ptr<Animation> pNewAnimation = std::make_unique<Animation>(m_pContext, pAnimation->mName.C_Str(), (int)m_Skeleton.BoneCount(), (float)pAnimation->mDuration, (float)pAnimation->mTicksPerSecond);
			
			AsyncTaskQueue* pQueue = GetSubsystem<AsyncTaskQueue>();
			for (unsigned int j = 0; j < pAnimation->mNumChannels; j++)
			{
				AsyncTask* pTask = pQueue->GetFreeTask();

				const aiNodeAnim* pAnimNode = pAnimation->mChannels[j];
				pTask->Action.BindLambda([this, &pNewAnimation, pAnimNode](AsyncTask*, int)
				{
					AnimationNode animNode;

					animNode.Name = pAnimNode->mNodeName.C_Str();
					auto pIt = m_BoneMap.find(animNode.Name);
					if (pIt == m_BoneMap.end())
					{
						FLUX_LOG(Warning, "[Mesh::ProcessAssimpAnimations] > There is no bone that matches the animation node '%s'", animNode.Name.c_str());
						return;
					}
					animNode.BoneIndex = pIt->second;

					std::vector<float> keyTimes;
					for (unsigned int k = 0; k < pAnimNode->mNumPositionKeys; k++)
						keyTimes.push_back((float)pAnimNode->mPositionKeys[k].mTime);
					for (unsigned int k = 0; k < pAnimNode->mNumScalingKeys; k++)
						keyTimes.push_back((float)pAnimNode->mScalingKeys[k].mTime);
					for (unsigned int k = 0; k < pAnimNode->mNumRotationKeys; k++)
						keyTimes.push_back((float)pAnimNode->mRotationKeys[k].mTime);
					std::sort(keyTimes.begin(), keyTimes.end());
					keyTimes.erase(std::unique(keyTimes.begin(), keyTimes.end()), keyTimes.end());

					for (float time : keyTimes)
					{
						AnimationKey key;
						key.Rotation = TxDXQuaternion(GetRotation(pAnimNode, time));
						key.Scale = ToDXVector3(GetScale(pAnimNode, time));
						key.Position = ToDXVector3(GetPosition(pAnimNode, time));
						animNode.Keys.push_back(std::pair<float, AnimationKey>(time, key));
					}
					pNewAnimation->SetNode(animNode);
				});

				pQueue->AddWorkItem(pTask);
			}
			pQueue->JoinAll();
			m_Animations.push_back(std::move(pNewAnimation));
		}
	}
	return true;
}

bool Mesh::ProcessSkeleton(const aiScene* pScene)
{
	AUTOPROFILE(Mesh_Load_Hierarchy);

	Matrix Identity = Matrix::CreateTranslation(0, 0, 0);
	ProcessNode(pScene->mRootNode, Identity);
	return true;
}

void Mesh::CalculateBoundingBox()
{
	AUTOPROFILE(Mesh_CalculateBoundingBox);

	for (auto& pGeometry : m_Geometries)
	{
		BoundingBox bb;
		BoundingBox::CreateFromPoints(bb, pGeometry->GetVertexCount(), (const XMFLOAT3*)pGeometry->GetVertexData("POSITION").pData, (int)sizeof(Vector3));
		m_BoundingBox.CreateMerged(m_BoundingBox, m_BoundingBox, bb);
	}
}

void Mesh::ProcessNode(aiNode* pNode, Matrix parentMatrix, Bone* pParentBone)
{
	Bone* pBone = m_Skeleton.GetBone(pNode->mName.C_Str());
	Matrix nodeTransform = ToDXMatrix(pNode->mTransformation);
	Matrix globalTransform = nodeTransform * parentMatrix;
	if (pBone)
	{
		if (m_Skeleton.GetParentBone() == nullptr)
			m_Skeleton.SetParentBone(pBone);
		pBone->pParent = pParentBone;
		if (pParentBone)
			pParentBone->Children.push_back(pBone);
		pParentBone = pBone;
	}
	
	for (unsigned int i = 0; i < pNode->mNumChildren; ++i)
	{
		ProcessNode(pNode->mChildren[i], globalTransform, pParentBone);
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
		return;

	char* pDataLocation = new char[vertexStride * pGeometry->GetVertexCount()];
	char* pVertexDataStart = pDataLocation;

	//Instead of getting the info every vertex, cache it in a local struct
	struct ElementInfo
	{
		ElementInfo(const VertexElement& element) :
			semanticName(VertexElement::GetSemanticOfType(element.Semantic)),
			elementSize(VertexElement::GetSizeOfType(element.Type))
		{ }
		std::string semanticName;
		unsigned int elementSize;
	};
	std::vector<ElementInfo> elementInfo;
	for (VertexElement& element : elementDesc)
		elementInfo.push_back(element);

	AsyncTaskQueue* pQueue = GetSubsystem<AsyncTaskQueue>();
	const int taskCount = 4;
	int vertexCountPerThread = pGeometry->GetVertexCount() / taskCount;
	int remaining = pGeometry->GetVertexCount() % taskCount;

	const std::map <std::string, Geometry::VertexData>& rawData = pGeometry->GetRawData();
	for (int i = 0; i < taskCount; ++i)
	{
		int vertexCount = vertexCountPerThread;
		if (i >= taskCount - 1)
			vertexCount += remaining;
		int startVertex = i * vertexCountPerThread;

		AsyncTask* pTask = pQueue->GetFreeTask();
		pTask->Action.BindLambda([pVertexDataStart, vertexCount, startVertex, rawData, elementInfo, vertexStride](AsyncTask* pTask, unsigned index)
		{
			UNREFERENCED_PARAMETER(pTask);
			UNREFERENCED_PARAMETER(index);
			char* pDataStart = (char*)pVertexDataStart + startVertex * vertexStride;
			for (int i = 0; i < vertexCount; i++)
			{
				int currentVertex = startVertex + i;
				for (size_t e = 0; e < elementInfo.size(); e++)
				{
					const ElementInfo& element = elementInfo[e];

					const void* pData = (const char*)rawData.at(element.semanticName).pData + element.elementSize * currentVertex;
					memcpy(pDataStart, pData, element.elementSize);
					pDataStart = (char*)pDataStart + element.elementSize;
				}
			}
		});
		pQueue->AddWorkItem(pTask);
	}
	if (pGeometry->HasData("INDEX"))
	{
		AsyncTask* pTask = pQueue->GetFreeTask();
		pTask->Action.BindLambda([&, this, pGeometry](AsyncTask* pTask, unsigned index)
		{
			UNREFERENCED_PARAMETER(pTask);
			UNREFERENCED_PARAMETER(index);

			std::unique_ptr<IndexBuffer> pIndexBuffer = std::make_unique<IndexBuffer>(GetSubsystem<Graphics>());
			pIndexBuffer->Create(pGeometry->GetIndexCount(), false, false);
			pIndexBuffer->SetData(pGeometry->GetVertexData("INDEX").pData);
			pGeometry->SetIndexBuffer(pIndexBuffer.get());
			m_IndexBuffers.push_back(std::move(pIndexBuffer));
		});
		pQueue->AddWorkItem(pTask);
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
	for (int i = 0; i < m_Geometries.size(); i++)
	{
		for (const auto& item : m_Geometries[i]->GetRawData())
		{
			memoryUsage += item.second.ByteSize();
		}
	}
	memoryUsage += (unsigned int)m_Skeleton.BoneCount() * sizeof(Bone);

	for (const auto& pAnimation : m_Animations)
	{
		for (const auto node : pAnimation->GetNodes())
		{
			memoryUsage += (unsigned int)node.Keys.size() * sizeof(AnimationKey);
		}
	}

	SetMemoryUsage(memoryUsage);
}