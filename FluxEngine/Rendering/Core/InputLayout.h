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

	void Create(const vector<VertexBuffer*>& vertexBuffers, ShaderVariation* pVariation);

	void* GetInputLayout() const { return m_pInputLayout; }

private:

	void* m_pInputLayout;
	Graphics* m_pGraphics;
};

