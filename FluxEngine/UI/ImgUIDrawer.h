#pragma once

class Graphics;
class VertexBuffer;
class IndexBuffer;
class InputLayout;
class Shader;
class ShaderVariation;
struct VertexElement;
class ConstantBuffer;

class ImgUIDrawer
{
public:
	ImgUIDrawer(Graphics* pGraphics);
	void Initialize();
	void Shutdown();
	void NewFrame();

	void Render();
	void OnResize();

	int WndProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void CreateFontsTexture();
	void LoadShader();


	ID3DX11EffectShaderResourceVariable* m_pTextureVariable = nullptr;
	ID3DX11EffectMatrixVariable* m_pViewProjVariable = nullptr;

	Unique_COM<ID3D11ShaderResourceView> m_pFontSRV;

	Graphics* m_pGraphics;
	vector<VertexElement> m_VertexElements;
	VertexBuffer* m_pVb = nullptr;
	IndexBuffer* m_pIb = nullptr;
	InputLayout* m_pIl = nullptr;

	ConstantBuffer* m_pConstantBuffer = nullptr;
	Shader* m_pShader = nullptr;
	ShaderVariation* m_pPixelShader = nullptr;
	ShaderVariation* m_pVertexShader = nullptr;
};