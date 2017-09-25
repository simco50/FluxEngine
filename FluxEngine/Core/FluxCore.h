#pragma once

class Graphics;
class Shader;
class ShaderVariation;
class VertexBuffer;
class InputLayout;
class ConstantBuffer;
class IndexBuffer;

class FluxCore
{
public:
	FluxCore();
	virtual ~FluxCore();

	DELETE_COPY(FluxCore)

	int Run(HINSTANCE hInstance);
	void GameLoop();
	void InitGame();

private:
	Graphics* m_pGraphics = nullptr;

	//Window variables
	HINSTANCE m_hInstance = nullptr;

	Shader* m_pShader;
	ShaderVariation* m_pVertexShader;
	ShaderVariation* m_pPixelShader;
	VertexBuffer* m_pVertexBuffer;
	InputLayout* m_pInputLayout;
	ConstantBuffer* m_pConstBuffer;
	IndexBuffer* m_pIndexBuffer;

	float m_DeltaTime = 0;
};
