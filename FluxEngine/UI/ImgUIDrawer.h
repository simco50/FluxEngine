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
	void OnResize();

	int WndProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void CreateFontsTexture();
	void LoadShader();

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
};