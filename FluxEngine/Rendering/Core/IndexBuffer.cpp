#include "FluxEngine.h"
#include "IndexBuffer.h"
#include "Graphics.h"

IndexBuffer::IndexBuffer(Graphics* pGraphics) :
	GraphicsObject(pGraphics)
{
}

IndexBuffer::~IndexBuffer()
{
	SafeRelease(m_pResource);
}