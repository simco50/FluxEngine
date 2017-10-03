#pragma once

class Graphics;
class Shader;
class ShaderVariation;
class VertexBuffer;
class InputLayout;
class ConstantBuffer;
class IndexBuffer;
class ImmediateUI;

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

	Shader* m_pShader = nullptr;
	ShaderVariation* m_pVertexShader = nullptr;
	ShaderVariation* m_pPixelShader = nullptr;
	VertexBuffer* m_pVertexBuffer = nullptr;
	InputLayout* m_pInputLayout = nullptr;
	ConstantBuffer* m_pConstBuffer = nullptr;
	IndexBuffer* m_pIndexBuffer = nullptr;

	ImmediateUI* m_pImmediateUI = nullptr;

	float m_DeltaTime = 0;
};
