#include "stdafx.h"
#include "MeshFilter.h"

#include "Rendering/Core/IndexBuffer.h"
#include "Rendering/Core/VertexBuffer.h"

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

	for (int i = 0; i < m_VertexCount; i++)
	{
		for (size_t e = 0; e < elementDesc.size(); e++)
		{
			string semanticName = VertexElement::GetSemanticOfType(elementDesc[e].Semantic);
			int elementSize = VertexElement::GetSizeOfType(elementDesc[e].Type);
			if (!HasData(semanticName))
			{
				//Only report a warning for the first element to prevent spam
				if (i == 0)
					FLUX_LOG(WARNING, "MeshFilter::CreateBuffers() > Material expects '%s' but mesh has no such data. Using dummy data", semanticName.c_str());
				//Get the stride of the required dummy data
				ZeroMemory(pDataLocation, elementSize);
				pDataLocation = (char*)pDataLocation + elementSize;
			}
			else
			{
				void* pData = (char*)GetVertexDataUnsafe(semanticName).pData + elementSize * i;
				memcpy(pDataLocation, pData, elementSize);
				pDataLocation = (char*)pDataLocation + elementSize;
			}
		}
	}

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
