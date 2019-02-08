#pragma once
#include "GraphicsResource.h"

class Graphics;

/*
	Register 0 is reserved for per-frame data
	Register 1 is reserved for per-view data
	Register 2 is reserved for per-object data
	Registers 3-8 is free to use

	See 'Rendering/Core/GraphicsDefines.h'
*/

class ConstantBuffer : public GraphicsResource
{
public:
	explicit ConstantBuffer(Graphics* pGraphics);
	~ConstantBuffer() = default;

	DELETE_COPY(ConstantBuffer)

	void SetSize(unsigned int size);
};