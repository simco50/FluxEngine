#include "FluxEngine.h"
#include "Mesh.h"

#include "Rendering/Core/IndexBuffer.h"
#include "Rendering/Core/VertexBuffer.h"
#include "FileSystem/File/PhysicalFile.h"
#include <thread>
#include "Geometry.h"

#include "Async/AsyncTaskQueue.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

bool Mesh::Load(const std::string& filePath)
{
	AUTOPROFILE_DESC(Mesh_Load, Paths::GetFileName(filePath));

	std::string extension = Paths::GetFileExtenstion(filePath);
	if (extension != "flux")
	{
		std::stringstream stream;
		stream << "MeshLoader::LoadContent() -> '" << filePath << "' has a wrong file extension";
		FLUX_LOG(ERROR, stream.str());
		return false;
	}

	std::unique_ptr<IFile> pFile = FileSystem::GetFile(filePath);
	if (pFile == nullptr)
		return false;
	if (!pFile->Open(FileMode::Read, ContentType::Binary))
		return false;

	m_MeshName = Paths::GetFileName(filePath);

	string magic = pFile->ReadSizedString();
	char minVersion, maxVersion;
	*pFile >> minVersion >> maxVersion;
	UNREFERENCED_PARAMETER(maxVersion);
	if (minVersion != MESH_VERSION)
	{
		std::stringstream stream;
		stream << "MeshLoader::LoadContent() File '" << filePath << "' version mismatch: Expects v" << MESH_VERSION << ".0 but is v" << (int)minVersion << ".0";
		FLUX_LOG(ERROR, stream.str());
	}

	pFile->Read(sizeof(BoundingBox), (char*)&m_BoundingBox);

	*pFile >> m_GeometryCount;

	for (int i = 0; i < m_GeometryCount; ++i)
	{
		std::unique_ptr<Geometry> pGeometry = std::make_unique<Geometry>();
		for (;;)
		{
			string block = pFile->ReadSizedString();
			for (char& c : block)
				c = (char)toupper(c);
			if (block == "ENDMESH")
				break;

			unsigned int length, stride;
			*pFile >> length >> stride;

			pGeometry->GetVertexDataUnsafe(block).pData = new char[length * stride];
			pGeometry->GetVertexDataUnsafe(block).Count = length;
			pGeometry->GetVertexDataUnsafe(block).Stride = stride;
			pFile->Read(length * stride, (char*)pGeometry->GetVertexDataUnsafe(block).pData);
		}

		pGeometry->SetDrawRange(PrimitiveType::TRIANGLELIST, pGeometry->GetDataCount("INDEX"), pGeometry->GetDataCount("POSITION"));
		m_Geometries.push_back(std::move(pGeometry));
	}

	pFile->Close();
	return true;
}

void Mesh::CreateBuffers(Graphics* pGraphics, vector<VertexElement>& elementDesc)
{
	AUTOPROFILE_DESC(Mesh_CreateBuffers, m_MeshName);
	for (std::unique_ptr<Geometry>& pGeometry : m_Geometries)
	{
		CreateBuffersForGeometry(pGraphics, elementDesc, pGeometry.get());
	}
}

void Mesh::CreateBuffersForGeometry(Graphics* pGraphics, vector<VertexElement>& elementDesc, Geometry* pGeometry)
{
	std::unique_ptr<VertexBuffer> pVertexBuffer = std::make_unique<VertexBuffer>(pGraphics);
	pVertexBuffer->Create(pGeometry->GetVertexCount(), elementDesc, false);

	int vertexStride = pVertexBuffer->GetVertexStride();
	if (vertexStride == 0)
	{
		FLUX_LOG(ERROR, "MeshFilter::CreateBuffers() > VertexStride of the InputLayout is 0");
		return;
	}

	char* pDataLocation = new char[vertexStride * pGeometry->GetVertexCount()];
	char* pVertexDataStart = pDataLocation;

#ifdef CACHE_MESHES
	unsigned long long hash = pVertexBuffer->GetBufferHash();
	stringstream str;
	str << "Resources\\Meshes\\Cache\\" << m_MeshName << "_" << hash;
	unique_ptr<IFile> pFile = FileSystem::GetFile(str.str());
	if (pFile)
	{
		pFile->Open(FileMode::Read, ContentType::Binary);
		pFile->Read(m_VertexCount * vertexStride, (char*)pVertexDataStart);
		pFile->Close();
	}
	else
	{
#endif
		//Instead of getting the info every vertex, cache it in a local struct
		struct ElementInfo
		{
			ElementInfo(const VertexElement& element)
			{
				semanticName = VertexElement::GetSemanticOfType(element.Semantic);
				elementSize = VertexElement::GetSizeOfType(element.Type);
			}
			string semanticName;
			unsigned int elementSize;
		};
		std::vector<ElementInfo> elementInfo;
		for (VertexElement& element : elementDesc)
			elementInfo.push_back(element);

		const int threadCount = 4;
		AsyncTaskQueue taskQueue(threadCount);
		int vertexCountPerThread = pGeometry->GetVertexCount() / threadCount;
		int remaining = pGeometry->GetVertexCount() % threadCount;

		const std::map <std::string, Geometry::VertexData>& rawData = pGeometry->GetRawData();
		for (int i = 0; i < threadCount; ++i)
		{
			int vertexCount = vertexCountPerThread;
			if (i >= threadCount - 1)
				vertexCount += remaining;
			int startVertex = i * vertexCountPerThread;

			AsyncTask* pTask = new AsyncTask();
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
			taskQueue.AddWorkItem(pTask);
		}

		taskQueue.JoinAll();


#ifdef CACHE_MESHES
		std::unique_ptr<PhysicalFile> pCacheFile = std::make_unique<PhysicalFile>(str.str());
		if (pCacheFile->Open(FileMode::Write, ContentType::Binary))
		{
			pCacheFile->Write((char*)pVertexDataStart, vertexStride * m_VertexCount);
			pCacheFile->Close();
		}
				}
#endif

	pVertexBuffer->SetData(pVertexDataStart);
	pGeometry->SetVertexBuffer(pVertexBuffer.get());
	m_VertexBuffers.push_back(std::move(pVertexBuffer));

	if (pGeometry->HasData("INDEX"))
	{
		std::unique_ptr<IndexBuffer> pIndexBuffer = std::make_unique<IndexBuffer>(pGraphics);
		pIndexBuffer->Create(pGeometry->GetIndexCount(), false, false);
		pIndexBuffer->SetData(pGeometry->GetVertexData("INDEX").pData);
		pGeometry->SetIndexBuffer(pIndexBuffer.get());
		m_IndexBuffers.push_back(std::move(pIndexBuffer));
	}

	m_BuffersInitialized = true;

	delete[] pVertexDataStart;
}

