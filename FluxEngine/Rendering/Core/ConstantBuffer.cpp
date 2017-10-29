#include "stdafx.h"
#include "ConstantBuffer.h"
#include "Graphics.h"

#ifdef D3D11
#include "D3D11/D3D11ConstantBuffer.hpp"
#endif

ConstantBuffer::ConstantBuffer(Graphics* pGraphics) : 
	m_pGraphics(pGraphics)
{

}

ConstantBuffer::~ConstantBuffer()
{
	Release();
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