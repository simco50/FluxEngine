#pragma once
#include "GraphicsObject.h"

class IndexBuffer : public GraphicsObject
{
public:
	explicit IndexBuffer(Graphics* pGraphics);
	~IndexBuffer();

	DELETE_COPY(IndexBuffer)

	void Create(int indexCount, bool smallIndexStride = false, bool dynamic = false);
	void SetData(void* pData);

	void* Map(bool discard);
	void Unmap();

	unsigned int GetCount() const { return m_IndexCount; }
	bool IsSmallStride() const { return m_SmallIndexStride; }

private:
	bool m_Dynamic = false;
	unsigned int m_IndexCount = 0;
	bool m_SmallIndexStride = false;
	bool m_HardwareLocked = false;
};

