#include "stdafx.h"
#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext):
	m_pDevice(pDevice),
	m_pDeviceContext(pDeviceContext)
{

}

VertexBuffer::~VertexBuffer()
{
}


void VertexBuffer::Create(const int vertexCount, const vector<VertexElement>& elements, bool dynamic)
{
	SetVertexSize(elements);

	m_VertexCount = vertexCount;

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = m_VertexStride * vertexCount;
	desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	
	HR(m_pDevice->CreateBuffer(&desc, nullptr, m_pBuffer.GetAddressOf()));
}

void* VertexBuffer::Map(bool discard)
{
	void* pBuffer = nullptr;

	D3D11_MAPPED_SUBRESOURCE mappedData;
	mappedData.pData = nullptr;

	HR(m_pDeviceContext->Map(m_pBuffer.Get(), 0, discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE, 0, &mappedData))
	pBuffer = mappedData.pData;

	m_HardwareLocked = true;
	return pBuffer;
}

void VertexBuffer::Unmap()
{
	if (m_HardwareLocked)
	{
		m_pDeviceContext->Unmap(m_pBuffer.Get(), 0);
		m_HardwareLocked = false;
	}
}

void VertexBuffer::SetVertexSize(const vector<VertexElement>& elements)
{
	m_VertexStride = 0;
	for (const VertexElement& element : elements)
	{
		switch (element.m_Type)
		{
		case VertexElementType::INT:
			m_VertexStride += 4;
			break;
		case VertexElementType::FLOAT:
			m_VertexStride += 4;
			break;
		case VertexElementType::VECTOR2:
			m_VertexStride += 8;
			break;
		case VertexElementType::VECTOR3:
			m_VertexStride += 12;
			break;
		case VertexElementType::VECTOR4:
			m_VertexStride += 16;
			break;
		case VertexElementType::UBYTE4:
			m_VertexStride += 4;
			break;
		case VertexElementType::UBYTE4_NORM:
			m_VertexStride += 4;
			break;
		case VertexElementType::MAX_VERTEX_ELEMENT_TYPES:
		default:
			Console::Log("Invalid vertex type!", LogType::ERROR);
			break;
		}
	}
	m_Elements = elements;
}
