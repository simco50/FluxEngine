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

	void* GetInputLayout() const { return m_pInputLayout.Get(); }

private:

	Smart_COM::Unique_COM<ID3D11InputLayout> m_pInputLayout;
	Graphics* m_pGraphics;
};

