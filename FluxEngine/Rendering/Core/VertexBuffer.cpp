#include "stdafx.h"
#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext):
	m_pDevice(pDevice),
	m_pDeviceContext(pDeviceContext)
{

}

VertexBuffer::~VertexBuffer()
{
	Release();
}

void VertexBuffer::Create(const int vertexCount, vector<VertexElement>& elements, bool dynamic)
{
	Release();

	SetVertexSize(elements);
	UpdateOffsets(elements);
	m_Elements = elements;

	m_VertexCount = vertexCount;

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = m_VertexStride * vertexCount;
	desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	
	HR(m_pDevice->CreateBuffer(&desc, nullptr, (ID3D11Buffer**)&m_pBuffer));
}

void VertexBuffer::SetData(void* pData)
{
	D3D11_BOX destBox;
	destBox.left = 0;
	destBox.right = m_VertexCount * m_VertexStride;
	destBox.top = 0;
	destBox.bottom = 1;
	destBox.front = 0;
	destBox.back = 1;

	m_pDeviceContext->UpdateSubresource((ID3D11Buffer*)m_pBuffer, 0, &destBox, pData, 0, 0);
}

void* VertexBuffer::Map(bool discard)
{
	void* pBuffer = nullptr;

	D3D11_MAPPED_SUBRESOURCE mappedData;
	mappedData.pData = nullptr;

	HR(m_pDeviceContext->Map((ID3D11Buffer*)m_pBuffer, 0, discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE, 0, &mappedData))
	pBuffer = mappedData.pData;

	m_HardwareLocked = true;
	return pBuffer;
}

void VertexBuffer::Unmap()
{
	if (m_HardwareLocked)
	{
		m_pDeviceContext->Unmap((ID3D11Buffer*)m_pBuffer, 0);
		m_HardwareLocked = false;
	}
}

void VertexBuffer::SetVertexSize(const vector<VertexElement>& elements)
{
	m_VertexStride = 0;
	for (const VertexElement& element : elements)
	{
		m_VertexStride += VertexElement::GetSizeOfType(element.Type);
	}
}

void VertexBuffer::UpdateOffsets(vector<VertexElement>& elements)
{
	unsigned int offset = 0;
	for (VertexElement& element : elements)
	{
		element.Offset = offset;
		offset += VertexElement::GetSizeOfType(element.Type);
	}
}

void VertexBuffer::Release()
{
	SafeRelease(m_pBuffer);
}