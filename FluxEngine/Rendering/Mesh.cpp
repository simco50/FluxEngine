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
	return ProcessAssimpMeshes(pScene);
}

bool Mesh::ProcessAssimpMeshes(const aiScene* pScene)
{
	if (pScene == nullptr)
		return false;

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
			m_BoneIndices.resize(pMesh->mNumVertices);
			m_BoneWeights.resize(pMesh->mNumVertices);
			for (unsigned int boneIndex = 0; boneIndex < pMesh->mNumBones; boneIndex++)
			{
				aiBone* pBone = pMesh->mBones[boneIndex];
				for (unsigned int weightIndex = 0; weightIndex < pBone->mNumWeights; weightIndex++)
				{
					const aiVertexWeight& pWeight = pBone->mWeights[weightIndex];
					AddWeight(pWeight.mVertexId, boneIndex, pWeight.mWeight);
				}
				m_Skeleton.Bones.push_back(Bone{ (int)boneIndex, pBone->mName.C_Str(), *reinterpret_cast<Matrix*>(&pBone->mOffsetMatrix) });
			}
		}
		m_Geometries.push_back(std::move(pGeometry));
	}
	return true;
}

bool Mesh::ProcessAssimpAnimations(const aiScene* pScene)
{
	UNREFERENCED_PARAMETER(pScene);
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

