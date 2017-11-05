#include "stdafx.h"
#include "MeshFilter.h"

#include "Rendering/Core/IndexBuffer.h"
#include "Rendering/Core/VertexBuffer.h"
#include "FileSystem/File/PhysicalFile.h"
#include <thread>

using namespace std;

MeshFilter::MeshFilter()
{
}

MeshFilter::~MeshFilter()
{
	for (VertexBuffer*& pVertexBuffer : m_VertexBuffers)
	{
		SafeDelete(pVertexBuffer);
	}
	for (auto &it : m_VertexData)
	{
		delete[] it.second.pData;
		it.second.pData = nullptr;
	}
}

void MeshFilter::CreateBuffers(Graphics* pGraphics, vector<VertexElement>& elementDesc)
{
	if (m_BuffersInitialized)
	{
		FLUX_LOG(ERROR, "MeshFilter::CreateBuffers() > Buffers are already initialized");
		return;
	}

	AUTOPROFILE(MeshFilter_CreateBuffers);

	for (VertexBuffer*& pVertexBuffer : m_VertexBuffers)
	{
		SafeDelete(pVertexBuffer);
	}
	m_pIndexBuffer.reset();

	VertexBuffer* pVertexBuffer = new VertexBuffer(pGraphics);
	m_VertexBuffers.push_back(pVertexBuffer);
	pVertexBuffer->Create(m_VertexCount, elementDesc, false);

	int vertexStride = pVertexBuffer->GetVertexStride();
	if (vertexStride == 0)
	{
		FLUX_LOG(ERROR, "MeshFilter::CreateBuffers() > VertexStride of the InputLayout is 0");
		return;
	}

	void* pDataLocation = new char[vertexStride * m_VertexCount];
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

		for (int i = 0; i < m_VertexCount; i++)
		{
			for (size_t e = 0; e < elementDesc.size(); e++)
			{
				const ElementInfo& element = elementInfo[e];
				if (!HasData(element.semanticName))
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
					const void* pData = (const char*)GetVertexDataUnsafe(element.semanticName).pData + element.elementSize * i;
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

	if (HasData("INDEX"))
	{
		m_pIndexBuffer = make_unique<IndexBuffer>(pGraphics);
		m_pIndexBuffer->Create(m_IndexCount, false, false);
		m_pIndexBuffer->SetData(GetVertexData("INDEX").pData);
	}

	m_BuffersInitialized = true;

	delete[] pVertexDataStart;
} 

VertexBuffer* MeshFilter::GetVertexBuffer(const unsigned int slot) const
{
	if (slot >= m_VertexBuffers.size())
	{
		FLUX_LOG(ERROR, "[MeshFilter::GetVertexBuffer] > No vertex buffer at slot %i", slot);
		return nullptr;
	}
	return m_VertexBuffers[slot];
}

MeshFilter::VertexData& MeshFilter::GetVertexData(const string& semantic)
{
	auto it = m_VertexData.find(semantic);
	if (it == m_VertexData.end())
		FLUX_LOG(ERROR, "MeshFilter::GetVertexData() > VertexData with semantic '%s' not found.", semantic.c_str());
	return it->second;
}

MeshFilter::VertexData& MeshFilter::GetVertexDataUnsafe(const string& semantic)
{
	return m_VertexData[semantic];
}

bool MeshFilter::HasData(const string& semantic) const
{
	auto it = m_VertexData.find(semantic);
	return !(it == m_VertexData.end());
}
