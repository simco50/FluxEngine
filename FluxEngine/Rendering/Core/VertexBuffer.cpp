#include "FluxEngine.h"
#include "VertexBuffer.h"
#include "Graphics.h"

VertexBuffer::VertexBuffer(Graphics* pGraphics)
	: GraphicsResource(pGraphics)
{
}

VertexBuffer::~VertexBuffer()
{
}

void VertexBuffer::SetVertexSize(const std::vector<VertexElement>& elements)
{
	m_ElementStride = 0;
	for (const VertexElement& element : elements)
	{
		m_ElementStride += VertexElement::GetSizeOfType(element.Type);
	}
}

void VertexBuffer::UpdateOffsets(std::vector<VertexElement>& elements)
{
	m_BufferHash = 0;

	unsigned int offset = 0;
	for (VertexElement& element : elements)
	{
		m_BufferHash <<= 10;
		m_BufferHash |= element.GetHash();
		element.Offset = offset;
		offset += VertexElement::GetSizeOfType(element.Type);
	}
}