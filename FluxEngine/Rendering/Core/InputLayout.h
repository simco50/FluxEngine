#pragma once
class VertexBuffer;
class ShaderVariation;

class InputLayout
{
public:
	InputLayout(ID3D11Device* pDevice);
	~InputLayout();

	DELETE_COPY(InputLayout)

	void Create(const vector<VertexBuffer*>& vertexBuffers, ShaderVariation* pVariation);

	void* GetInputLayout() const { return m_pInputLayout.Get(); }

private:

	Smart_COM::Unique_COM<ID3D11InputLayout> m_pInputLayout;
	ID3D11Device* m_pDevice;
};

