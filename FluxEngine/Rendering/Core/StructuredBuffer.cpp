#include "FluxEngine.h"
#include "StructuredBuffer.h"
#include "Graphics.h"

StructuredBuffer::StructuredBuffer(Graphics* pGraphics)
	: GraphicsObject(pGraphics)
{

}

StructuredBuffer::~StructuredBuffer()
{
	Release();
}

void StructuredBuffer::SetElement_Internal(const size_t index, const void* pElement)
{
	assert(index < m_ElementCount);
	memcpy((char*)m_pMappedData + index * m_Stride, pElement, m_Stride);
}
