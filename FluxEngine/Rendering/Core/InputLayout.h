#pragma once
class VertexBuffer;
class ShaderVariation;
class Graphics;

class InputLayout
{
public:
	InputLayout(Graphics* pGraphics);
	~InputLayout();

	DELETE_COPY(InputLayout)

	void Create(std::vector<VertexBuffer*> vertexBuffers, ShaderVariation* pVariation);
	void Create(VertexBuffer** vertexBuffers, unsigned int bufferCount, ShaderVariation* pVariation);

	void* GetInputLayout() const { return m_pInputLayout; }

private:

	void* m_pInputLayout = nullptr;
	Graphics* m_pGraphics;
};

