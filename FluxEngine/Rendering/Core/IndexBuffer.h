#pragma once
#include "GraphicsResource.h"

class IndexBuffer : public GraphicsResource
{
public:
	explicit IndexBuffer(Graphics* pGraphics);
	~IndexBuffer();

	DELETE_COPY(IndexBuffer)

	void Create(int indexCount, bool smallIndexStride = false, bool dynamic = false);
};