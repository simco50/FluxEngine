#include "stdafx.h"
#include "ConstantBuffer.h"


ConstantBuffer::ConstantBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext) :
	m_pDevice(pDevice),
	m_pDeviceContext(pDeviceContext)
{
}

ConstantBuffer::~ConstantBuffer()
{
}

void ConstantBuffer::SetSize(const unsigned int size)
{
	Release();

	m_pShadowData = new unsigned char[size];

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = size;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	
	HR(m_pDevice->CreateBuffer(&desc, nullptr, &m_pBuffer));
}

void ConstantBuffer::Apply()
{
	if (m_IsDirty && m_pBuffer)
	{
		m_pDeviceContext->UpdateSubresource(m_pBuffer, 0, 0, (void*)m_pShadowData, 0, 0);
		m_IsDirty = false;
	}
}

void ConstantBuffer::SetParameter(unsigned char offset, const unsigned char size, const void* pData)
{
	memcpy(&m_pShadowData[offset], pData, size);
}

void ConstantBuffer::Release()
{
	SafeRelease(m_pBuffer);
	if (m_pShadowData != nullptr)
	{
		delete[] m_pShadowData;
		m_pShadowData = nullptr;
	}
}
