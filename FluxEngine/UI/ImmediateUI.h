#pragma once

class Graphics;
class VertexBuffer;
class IndexBuffer;
class Shader;
class ShaderVariation;
struct VertexElement;
class ConstantBuffer;
class Texture;
class InputEngine;

class ImmediateUI
{
public:
	ImmediateUI(Graphics* pGraphics, InputEngine* pInput);
	~ImmediateUI();

	DELETE_COPY(ImmediateUI)

	void NewFrame();
	void Render();
	static int WndProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	Graphics* m_pGraphics;
	InputEngine* m_pInput;

	vector<VertexElement> m_VertexElements;

	unique_ptr<Texture> m_pFontTexture;
	unique_ptr<VertexBuffer> m_pVertexBuffer;
	unique_ptr<IndexBuffer> m_pIndexBuffer;
	unique_ptr<Shader> m_pShader;
	ShaderVariation* m_pPixelShader = nullptr;
	ShaderVariation* m_pVertexShader = nullptr;

	static const int START_VERTEX_COUNT = 1000;
	static const int START_INDEX_COUNT = 1000;
};