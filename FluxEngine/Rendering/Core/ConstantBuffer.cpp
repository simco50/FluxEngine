#include "stdafx.h"
#include "ConstantBuffer.h"
#include "Graphics.h"

ConstantBuffer::ConstantBuffer(Graphics* pGraphics) : 
	m_pGraphics(pGraphics)
{

}

ConstantBuffer::~ConstantBuffer()
{
	Release();
}

void ConstantBuffer::SetSize(const unsigned int size)
{
	Release();
	m_Size = size;

	m_pShadowData = new unsigned char[m_Size];

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = m_Size;
	desc.CPUAccessFlags = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	
	HR(m_pGraphics->GetDevice()->CreateBuffer(&desc, nullptr, (ID3D11Buffer**)&m_pBuffer));
}

void ConstantBuffer::Apply()
{
	if (m_IsDirty && m_pBuffer)
	{
		m_pGraphics->GetDeviceContext()->UpdateSubresource((ID3D11Buffer*)m_pBuffer, 0, 0, (void*)m_pShadowData, 0, 0);
		m_IsDirty = false;
	}
}

bool ConstantBuffer::SetParameter(unsigned int offset, const unsigned int size, const void* pData)
{
	if (m_Size < offset + size)
		return false;
	memcpy(&m_pShadowData[offset], pData, size);
	m_IsDirty = true;
	return true;
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