#include "stdafx.h"
#include "IndexBuffer.h"
#include "Graphics.h"

#ifdef D3D11
#include "D3D11/D3D11IndexBuffer.hpp"
#endif

IndexBuffer::IndexBuffer(Graphics* pGraphics) :
	m_pGraphics(pGraphics)
{

}

IndexBuffer::~IndexBuffer()
{
	SafeRelease(m_pBuffer);
}