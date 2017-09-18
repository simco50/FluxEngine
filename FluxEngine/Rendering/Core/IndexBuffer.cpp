#include "stdafx.h"
#include "IndexBuffer.h"


IndexBuffer::IndexBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext):
	m_pDevice(pDevice),
	m_pDeviceContext(pDeviceContext)
{

}

IndexBuffer::~IndexBuffer()
{
}

void IndexBuffer::Create(const int indexCount, bool dynamic /*= false*/)
{
	m_IndexCount = indexCount;
	m_Dynamic = dynamic;

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.ByteWidth = indexCount * sizeof(unsigned int);
	desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	HR(m_pDevice->CreateBuffer(&desc, nullptr, m_pBuffer.GetAddressOf()));
}

void* IndexBuffer::Map(bool discard)
{
	void* pBuffer = nullptr;

	D3D11_MAPPED_SUBRESOURCE mappedData;
	mappedData.pData = nullptr;

	HR(m_pDeviceContext->Map(m_pBuffer.Get(), 0, discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE, 0, &mappedData))
		pBuffer = mappedData.pData;

	m_HardwareLocked = true;
	return pBuffer;
}

void IndexBuffer::Unmap()
{
	if (m_HardwareLocked)
	{
		m_pDeviceContext->Unmap(m_pBuffer.Get(), 0);
		m_HardwareLocked = false;
	}
}
