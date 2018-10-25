#pragma once
#include "GraphicsObject.h"
class VertexBuffer;
class ShaderVariation;

class InputLayout : public GraphicsObject
{
public:
	InputLayout(Graphics* pGraphics);
	~InputLayout();

	DELETE_COPY(InputLayout)

	void Create(std::vector<VertexBuffer*> vertexBuffers, ShaderVariation* pVariation);
	void Create(VertexBuffer** vertexBuffers, unsigned int bufferCount, ShaderVariation* pVariation);
};