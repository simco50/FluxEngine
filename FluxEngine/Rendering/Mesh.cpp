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

aiQuaternion GetRotation(const aiNodeAnim* pNode, const float time)
{
	if (pNode->mNumRotationKeys == 0)
		return aiQuaternion();
	aiQuaternion quat = pNode->mRotationKeys[0].mValue;
	for (unsigned int i = 0; i < pNode->mNumRotationKeys; i++)
	{
		if (pNode->mRotationKeys[i].mTime == time)
			return pNode->mRotationKeys[i].mValue;
		if (pNode->mRotationKeys[i].mTime > time)
		{
			aiQuaternion output;
			aiQuaterniont<float>::Interpolate(output, quat, pNode->mRotationKeys[i].mValue, (float)(pNode->mRotationKeys[i].mTime - time));
			return output;
		}
		quat = pNode->mRotationKeys[i].mValue;
	}
	return quat;
}

aiVector3D GetScale(const aiNodeAnim* pNode, const float time)
{
	if (pNode->mNumScalingKeys == 0)
		return aiVector3D();
	aiVector3D scale = pNode->mScalingKeys[0].mValue;
	for (unsigned int i = 1; i < pNode->mNumScalingKeys; i++)
	{
		if (pNode->mScalingKeys[i].mTime > time)
		{
			return scale + (pNode->mScalingKeys[i].mValue - scale) * (float)(pNode->mScalingKeys[i].mTime - time);
		}
		scale = pNode->mScalingKeys[i].mValue;
	}
	return scale;
}

aiVector3D GetPosition(const aiNodeAnim* pNode, const float time)
{
	if (pNode->mNumPositionKeys == 0)
		return aiVector3D();
	aiVector3D pos = pNode->mPositionKeys[0].mValue;
	for (unsigned int i = 1; i < pNode->mNumPositionKeys; i++)
	{
		if (pNode->mPositionKeys[i].mTime > time)
		{
			return pos + (pNode->mPositionKeys[i].mValue - pos) * (float)(pNode->mPositionKeys[i].mTime - time);
		}
		pos = pNode->mPositionKeys[i].mValue;
	}
	return pos;
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
		return LoadFlux(inputStream);
	}
	FLUX_LOG(Warning, "[Mesh::Load] Slow loading '%s' using Assimp", fileName.c_str());
	return LoadAssimp(inputStream);
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
	std::vector<unsigned char> buffer;
	inputStream.ReadAllBytes(buffer);

	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFileFromMemory(buffer.data(), buffer.size(),
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded |
		aiProcess_GenSmoothNormals
	);

	if (pScene == nullptr)
		return false;
	if (!ProcessAssimpMeshes(pScene))
		return false;
	if (!ProcessAssimpAnimations(pScene))
		return false;
	return true;
}

bool Mesh::ProcessAssimpMeshes(const aiScene* pScene)
{
	m_GeometryCount = pScene->mNumMeshes;
	for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
	{
		aiMesh* pMesh = pScene->mMeshes[i];
		std::unique_ptr<Geometry> pGeometry = std::make_unique<Geometry>();
		pGeometry->SetDrawRange(PrimitiveType::TRIANGLELIST, pMesh->mNumVertices, pMesh->mNumFaces * 3);

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
				for (unsigned int weightIndex = 0; weightIndex < pBone->mNumWeights; weightIndex++)
				{
					const aiVertexWeight& pWeight = pBone->mWeights[weightIndex];
					for (int j = 0; j < 4; j++)
					{
						if (pWeights[pWeight.mVertexId * 4 + j] == 0.0f)
						{
							pWeights[pWeight.mVertexId * 4 + j] = pWeight.mWeight;
							pIndices[pWeight.mVertexId * 4 + j] = boneIndex;
							break;
						}
					}
				}

				Matrix a = Matrix::CreateTranslation(Vector3(2, 4, 6));
				aiMatrix4x4 b;
				aiMatrix4x4t<float>::Translation(aiVector3D(2, 4, 6), b);
				m_Skeleton.AddBone(Bone{ (int)boneIndex, pBone->mName.C_Str(), *reinterpret_cast<Matrix*>(&pBone->mOffsetMatrix) });
			}
		}
		m_Geometries.push_back(std::move(pGeometry));
	}
	return true;
}

bool Mesh::ProcessAssimpAnimations(const aiScene* pScene)
{
	if (pScene->HasAnimations())
	{
		for (unsigned int i = 0; i < pScene->mNumAnimations; i++)
		{
			const aiAnimation* pAnimation = pScene->mAnimations[i];
			Animation animation(pAnimation->mName.C_Str(), pAnimation->mNumChannels, (float)pAnimation->mDuration, (float)pAnimation->mTicksPerSecond);
			for (unsigned int j = 0; j < pAnimation->mNumChannels; j++)
			{
				AnimationNode animNode;
				aiNodeAnim* pAnimNode = pAnimation->mChannels[j];
				const Bone* pBone = m_Skeleton.GetBone(pAnimNode->mNodeName.C_Str());
				if (pBone == nullptr)
					continue;

				animNode.Name = pAnimNode->mNodeName.C_Str();
				animNode.BoneIndex = pBone->Index;

				std::map<float, aiMatrix4x4> keys;
				for (unsigned int k = 0; k < pAnimNode->mNumPositionKeys; k++)
					keys[(float)pAnimNode->mPositionKeys[k].mTime] = aiMatrix4x4();
				for (unsigned int k = 0; k < pAnimNode->mNumScalingKeys; k++)
					keys[(float)pAnimNode->mScalingKeys[k].mTime] = aiMatrix4x4();
				for (unsigned int k = 0; k < pAnimNode->mNumRotationKeys; k++)
					keys[(float)pAnimNode->mRotationKeys[k].mTime] = aiMatrix4x4();

				for (auto& key : keys)
				{
					aiQuaternion quat = GetRotation(pAnimNode, key.first);
					aiVector3D scale = GetScale(pAnimNode, key.first);
					aiVector3D position = GetPosition(pAnimNode, key.first);
					key.second = *reinterpret_cast<const aiMatrix4x4*>(&pBone->OffsetMatrix) * aiMatrix4x4(scale, quat, position);
				}

				for (const auto& key : keys)
				{
					aiVector3D scaling, translation;
					aiQuaternion rotation;
					key.second.Decompose(scaling, rotation, translation);
					AnimationKey animationKey;
					animationKey.Position = *reinterpret_cast<Vector3*>(&translation);
					animationKey.Scale = *reinterpret_cast<Vector3*>(&scaling);
					animationKey.Rotation = *reinterpret_cast<Quaternion*>(&rotation);
					animNode.Keys.push_back(std::pair<float, AnimationKey>(key.first, animationKey));
				}
				animation.SetNode(pBone->Index, animNode);
			}
			m_Animations.push_back(animation);
		}
	}
	return true;
}

void Mesh::CreateBuffersForGeometry(std::vector<VertexElement>& elementDesc, Geometry* pGeometry)
{
	std::unique_ptr<VertexBuffer> pVertexBuffer = std::make_unique<VertexBuffer>(GetSubsystem<Graphics>());
	pVertexBuffer->Create(pGeometry->GetVertexCount(), elementDesc, false);

	int vertexStride = pVertexBuffer->GetVertexStride();
	if (vertexStride == 0)
	{
		FLUX_LOG(Error, "MeshFilter::CreateBuffers() > VertexStride of the InputLayout is 0");
		return;
	}

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

	m_BuffersInitialized = true;
	delete[] pVertexDataStart;
}