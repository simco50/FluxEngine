#pragma once

class Graphics;
class Shader;
class ShaderVariation;
class VertexBuffer;
class ConstantBuffer;
class IndexBuffer;
class ImmediateUI;
class InputEngine;
class FreeCamera;

class FluxCore
{
public:
	FluxCore();
	virtual ~FluxCore();

	DELETE_COPY(FluxCore)

	int Run(HINSTANCE hInstance);
	void GameLoop();

	void RenderUI();

	void InitGame();

private:
	//Window variables
	HINSTANCE m_hInstance = nullptr;

	Shader* m_pShader = nullptr;
	ShaderVariation* m_pVertexShader = nullptr;
	ShaderVariation* m_pPixelShader = nullptr;
	VertexBuffer* m_pVertexBuffer = nullptr;
	VertexBuffer* m_pInstanceBuffer = nullptr;
	IndexBuffer* m_pIndexBuffer = nullptr;

	Graphics* m_pGraphics = nullptr;
	unique_ptr<ImmediateUI> m_pImmediateUI;
	unique_ptr<InputEngine> m_pInput;

	float m_DeltaTime = 0;
	XMFLOAT4 m_Color = XMFLOAT4(1, 1, 1, 1);
	XMFLOAT3 m_LightDirection = XMFLOAT3(-0.577f, -0.577f, 0.577f);

	int m_IndexCount = -1;

	unique_ptr<FreeCamera> m_pCamera;
};
