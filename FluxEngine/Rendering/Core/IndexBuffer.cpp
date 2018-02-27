#include "FluxEngine.h"
#include "IndexBuffer.h"
#include "Graphics.h"

IndexBuffer::IndexBuffer(Graphics* pGraphics) :
	m_pGraphics(pGraphics)
{
}

IndexBuffer::~IndexBuffer()
{
	SafeRelease(m_pBuffer);
}