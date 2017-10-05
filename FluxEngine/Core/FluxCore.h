#pragma once

class Graphics;
class Shader;
class ShaderVariation;
class VertexBuffer;
class InputLayout;
class ConstantBuffer;
class IndexBuffer;
class ImmediateUI;
class InputEngine;
class Context;

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


	//Window variables
	HINSTANCE m_hInstance = nullptr;

	Shader* m_pShader = nullptr;
	ShaderVariation* m_pVertexShader = nullptr;
	ShaderVariation* m_pPixelShader = nullptr;
	VertexBuffer* m_pVertexBuffer = nullptr;
	InputLayout* m_pInputLayout = nullptr;
	ConstantBuffer* m_pConstBuffer = nullptr;
	IndexBuffer* m_pIndexBuffer = nullptr;

	unique_ptr<Graphics> m_pGraphics;
	unique_ptr<Context> m_pContext;
	unique_ptr<ImmediateUI> m_pImmediateUI;
	unique_ptr<InputEngine> m_pInput;

	float m_DeltaTime = 0;
	XMFLOAT4 color = XMFLOAT4(0,0,0,1);
};
