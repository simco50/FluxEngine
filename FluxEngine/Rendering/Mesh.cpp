#include "stdafx.h"
#include "Mesh.h"

#include "Rendering/Core/IndexBuffer.h"
#include "Rendering/Core/VertexBuffer.h"
#include "FileSystem/File/PhysicalFile.h"
#include <thread>
#include "Geometry.h"

using namespace std;

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

bool Mesh::Load(const std::string& filePath)
{
	std::string extension = Paths::GetFileExtenstion(filePath);
	if (extension != "flux")
	{
		stringstream stream;
		stream << "MeshLoader::LoadContent() -> '" << filePath << "' has a wrong file extension";
		FLUX_LOG(ERROR, stream.str());
		return false;
	}

	unique_ptr<IFile> pFile = FileSystem::GetFile(filePath);
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
		stringstream stream;
		stream << "MeshLoader::LoadContent() File '" << filePath << "' version mismatch: Expects v" << MESH_VERSION << ".0 but is v" << (int)minVersion << ".0";
		FLUX_LOG(ERROR, stream.str());
	}

	*pFile >> m_GeometryCount;

	for (int i = 0; i < m_GeometryCount; ++i)
	{
		unique_ptr<Geometry> pGeometry = make_unique<Geometry>();
		for (;;)
		{
			string block = pFile->ReadSizedString();
			for (char& c : block)
				c = (char)toupper(c);
			if (block == "END")
				break;
			if (block == "ENDMESH")
				break;

			unsigned int length, stride;
			*pFile >> length >> stride;

			pGeometry->GetVertexDataUnsafe(block).pData = new char[length * stride];
			pGeometry->GetVertexDataUnsafe(block).Count = length;
			pGeometry->GetVertexDataUnsafe(block).Stride = stride;
			pFile->Read(length * stride, (char*)pGeometry->GetVertexDataUnsafe(block).pData);
		}

		pGeometry->m_VertexCount = pGeometry->GetVertexData("POSITION").Count;
		pGeometry->m_IndexCount = pGeometry->GetVertexData("INDEX").Count;

		m_Geometries.push_back(std::move(pGeometry));
	}

	pFile->Close();
	return true;
}

void Mesh::CreateBuffers(Graphics* pGraphics, vector<VertexElement>& elementDesc)
{
	AUTOPROFILE(Mesh_CreateBuffers);
	for (unique_ptr<Geometry>& pGeometry : m_Geometries)
	{
		CreateBuffersForGeometry(pGraphics, elementDesc, pGeometry.get());
	}
}

void Mesh::CreateBuffersForGeometry(Graphics* pGraphics, vector<VertexElement>& elementDesc, Geometry* pGeometry)
{
	unique_ptr<VertexBuffer> pVertexBuffer = make_unique<VertexBuffer>(pGraphics);
	pVertexBuffer->Create(pGeometry->GetVertexCount(), elementDesc, false);

	int vertexStride = pVertexBuffer->GetVertexStride();
	if (vertexStride == 0)
	{
		FLUX_LOG(ERROR, "MeshFilter::CreateBuffers() > VertexStride of the InputLayout is 0");
		return;
	}

	void* pDataLocation = new char[vertexStride * pGeometry->GetVertexCount()];
	void* pVertexDataStart = pDataLocation;

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
		vector<ElementInfo> elementInfo;
		for (VertexElement& element : elementDesc)
			elementInfo.push_back(element);

#if THREADING // Threading test
		class MeshThread
		{
		public:
			MeshThread(const vector<ElementInfo>& info, map<string, VertexData>& pData, void* pStart, int startVertex, int vertexStride, int count) :
				Info(info), pDataStart(pStart), VertexCount(count), pVertexData(pData), t{}, StartVertex(startVertex)
			{
				pDataStart = (char*)pDataStart + startVertex * vertexStride;
			}

			void Start()
			{
				t = thread{ [this]()
				{
					for (int i = 0; i < VertexCount; i++)
					{
						int currentVertex = StartVertex + i;
						for (size_t e = 0; e < Info.size(); e++)
						{
							const ElementInfo& element = Info[e];

							const void* pData = (const char*)pVertexData[element.semanticName].pData + element.elementSize * currentVertex;
							memcpy(pDataStart, pData, element.elementSize);
							pDataStart = (char*)pDataStart + element.elementSize;
						}
					}
				} };
			}

			void Wait()
			{
				if (t.joinable())
					t.join();
			}
		private:
			const vector<ElementInfo>& Info;
			int StartVertex;
			void* pDataStart;
			int VertexCount;
			std::thread t;
			map<string, VertexData>& pVertexData;
		};

		const int threadCount = 4;
		int vertexCountPerThread = m_VertexCount / threadCount;
		int remaining = m_VertexCount % threadCount;
		vector<MeshThread> meshThreads;
		for (int i = 0; i < threadCount; ++i)
		{
			int vertexCount = vertexCountPerThread;
			if (i >= threadCount - 1)
				vertexCount += remaining;
			int startVertex = i * vertexCountPerThread;
			meshThreads.push_back(MeshThread(elementInfo, m_VertexData, (char*)pVertexDataStart, startVertex, vertexStride, vertexCount));
		}
		for (int i = 0; i < threadCount; ++i)
			meshThreads[i].Start();
		for (int i = 0; i < threadCount; ++i)
			meshThreads[i].Wait();

#else

		for (int i = 0; i < pGeometry->GetVertexCount(); i++)
		{
			for (size_t e = 0; e < elementDesc.size(); e++)
			{
				const ElementInfo& element = elementInfo[e];
				if (!pGeometry->HasData(element.semanticName))
				{
					//Only report a warning for the first element to prevent spam
					if (i == 0)
						FLUX_LOG(WARNING, "MeshFilter::CreateBuffers() > Material expects '%s' but mesh has no such data. Using dummy data", element.semanticName.c_str());
					//Get the stride of the required dummy data
					ZeroMemory(pDataLocation, element.elementSize);
					pDataLocation = (char*)pDataLocation + element.elementSize;
				}
				else
				{
					const void* pData = (const char*)pGeometry->GetVertexDataUnsafe(element.semanticName).pData + element.elementSize * i;
					memcpy(pDataLocation, pData, element.elementSize);
					pDataLocation = (char*)pDataLocation + element.elementSize;
				}
			}
		}
#endif

#ifdef CACHE_MESHES
		unique_ptr<PhysicalFile> pCacheFile = make_unique<PhysicalFile>(str.str());
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
		unique_ptr<IndexBuffer> pIndexBuffer = make_unique<IndexBuffer>(pGraphics);
		pIndexBuffer = make_unique<IndexBuffer>(pGraphics);
		pIndexBuffer->Create(pGeometry->GetIndexCount(), false, false);
		pIndexBuffer->SetData(pGeometry->GetVertexData("INDEX").pData);
		pGeometry->SetIndexBuffer(pIndexBuffer.get());
		m_IndexBuffers.push_back(std::move(pIndexBuffer));
	}

	m_BuffersInitialized = true;

	delete[] pVertexDataStart;
}

