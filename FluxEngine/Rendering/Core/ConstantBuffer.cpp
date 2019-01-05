#include "FluxEngine.h"
#include "ConstantBuffer.h"
#include "Graphics.h"

ConstantBuffer::ConstantBuffer(Graphics* pGraphics)
	: GraphicsObject(pGraphics)
{

}

ConstantBuffer::~ConstantBuffer()
{
	Release();
}

bool ConstantBuffer::SetParameter(unsigned int offset, const unsigned int size, const void* pData)
{
	check(m_pShadowData);
	checkf(m_Size >= offset + size, "Trying to modify data outside of bounds!");
	memcpy(&m_pShadowData[offset], pData, size);
	m_IsDirty = true;
	return true;
}

void ConstantBuffer::Release()
{
	SafeRelease(m_pResource);
	if (m_pShadowData != nullptr)
	{
		delete[] m_pShadowData;
		m_pShadowData = nullptr;
	}
}