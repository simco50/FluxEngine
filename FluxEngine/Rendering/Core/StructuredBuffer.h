#pragma once
#include "GraphicsResource.h"

class StructuredBuffer : public GraphicsResource
{
public:
	explicit StructuredBuffer(Graphics* pGraphics);
	~StructuredBuffer();

	DELETE_COPY(StructuredBuffer)

	void Create(int elementCount, int elementStride, bool dynamic = false);
};