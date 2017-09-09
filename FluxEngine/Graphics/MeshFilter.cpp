#include "stdafx.h"
#include "MeshFilter.h"

using namespace std;

MeshFilter::MeshFilter()
{
}

MeshFilter::~MeshFilter()
{
	delete[] m_pVertexDataStart;
	for (auto &it : m_VertexData)
	{
		delete[] it.second.pData;
		it.second.pData = nullptr;
	}
}

void MeshFilter::Initialize(GameContext* pGameContext)
{
	m_pGameContext = pGameContext;
}

void MeshFilter::CreateBuffers(const InputLayoutDesc* ILDesc)
{
	if (m_BuffersInitialized)
		return;

	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	int vertexStride = ILDesc->VertexStride;
	if (ILDesc->VertexStride == 0)
	{
		Console::Log("MeshFilter::CreateBuffers() > VertexStride of the InputLayout is 0", LogType::ERROR);
		return;
	}

	void* pDataLocation = new char[vertexStride * m_VertexCount];
	m_pVertexDataStart = pDataLocation;

	for (int i = 0; i < m_VertexCount; i++)
	{
		for (size_t e = 0; e < ILDesc->LayoutDesc.size(); e++)
		{
			if (m_VertexData.find(ILDesc->LayoutDesc[e].SemanticName) == m_VertexData.end())
			{
				if (i == 0)
				{
					string m = string(ILDesc->LayoutDesc[e].SemanticName);
					wstring msg = wstring(m.begin(), m.end());

					Console::LogFormat(LogType::WARNING, "MeshFilter::CreateBuffers() > Material expects '%s' but mesh has no such data. Using dummy data",  msg.c_str());
				}
				//Get the stride of the required dummy data
				int size;
				if (e == ILDesc->LayoutDesc.size() - 1)
					size = vertexStride - ILDesc->LayoutDesc[e].AlignedByteOffset;
				else
					size = ILDesc->LayoutDesc[e + 1].AlignedByteOffset - ILDesc->LayoutDesc[e].AlignedByteOffset;
				vector<char> dummy(size, 0);
				memcpy(pDataLocation, dummy.data(), size);
				pDataLocation = (char*)pDataLocation + size;
			}
			else
			{
				int stride = GetVertexDataUnsafe(ILDesc->LayoutDesc[e].SemanticName).Stride;
				void* pData = (char*)GetVertexDataUnsafe(ILDesc->LayoutDesc[e].SemanticName).pData + stride * i;
				memcpy(pDataLocation, pData, stride);
				pDataLocation = (char*)pDataLocation + stride;
			}
		}
	}

	int bufferSize = ((char*)pDataLocation - (char*)m_pVertexDataStart);
	//Create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.ByteWidth = bufferSize;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = m_pVertexDataStart;

	HR(m_pGameContext->Engine->D3Device->CreateBuffer(&bd, &initData, &m_pVertexBuffer));

	//Create the index buffer
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.ByteWidth = m_IndexCount * sizeof(DWORD);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.Usage = D3D11_USAGE_IMMUTABLE;

	vector<DWORD> indices(m_IndexCount);
	memcpy(indices.data(), GetVertexData("INDEX").pData, m_IndexCount * sizeof(DWORD));
	initData.pSysMem = indices.data();
	HR(m_pGameContext->Engine->D3Device->CreateBuffer(&bd, &initData, &m_pIndexBuffer))

	m_BuffersInitialized = true;
}

MeshFilter::VertexData& MeshFilter::GetVertexData(const string& semantic)
{
	auto it = m_VertexData.find(semantic);
	if (it == m_VertexData.end())
		Console::LogFormat(LogType::ERROR, "MeshFilter::GetVertexData() > VertexData with semantic '%s' not found.", semantic.c_str());
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
