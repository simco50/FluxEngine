#pragma once

class Graphics;
class VertexBuffer;
class IndexBuffer;
class InputLayout;
class Shader;
class ShaderVariation;
struct VertexElement;
class ConstantBuffer;
class Texture;

class ImgUIDrawer
{
public:
	ImgUIDrawer(Graphics* pGraphics);
	~ImgUIDrawer();

	void NewFrame();

	void Render();

	int WndProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	Graphics* m_pGraphics;
	vector<VertexElement> m_VertexElements;

	Texture* m_pFontTexture = nullptr;
	VertexBuffer* m_pVertexBuffer = nullptr;
	IndexBuffer* m_pIndexBuffer = nullptr;
	InputLayout* m_pInputLayout = nullptr;
	ConstantBuffer* m_pConstantBuffer = nullptr;
	Shader* m_pShader = nullptr;
	ShaderVariation* m_pPixelShader = nullptr;
	ShaderVariation* m_pVertexShader = nullptr;

	static const int START_VERTEX_COUNT = 1000;
	static const int START_INDEX_COUNT = 1000;
};