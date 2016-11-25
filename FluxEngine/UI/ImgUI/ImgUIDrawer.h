#pragma once

class ImgUIDrawer
{
public:
	ImgUIDrawer();
	~ImgUIDrawer();

	void Initialize(EngineContext* pEngineContext);
	void Shutdown();
	void NewFrame();

	void Render();
	void OnResize();

private:
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void CreateFontsTexture();
	void LoadShader();

	EngineContext* m_pEngineContext = nullptr;

	unsigned int m_VertexBufferSize = 1000;
	unsigned int m_IndexBufferSize = 1000;
	Unique_COM<ID3D11Buffer> m_pVertexBuffer;
	Unique_COM<ID3D11Buffer> m_pIndexBuffer;
	Unique_COM<ID3D11InputLayout> m_pInputLayout;
	ID3DX11Effect* m_pEffect = nullptr;
	ID3DX11EffectTechnique* m_pTechnique = nullptr;
	
	ID3DX11EffectShaderResourceVariable* m_pTextureVariable = nullptr;
	ID3DX11EffectMatrixVariable* m_pViewProjVariable = nullptr;

	Unique_COM<ID3D11ShaderResourceView> m_pFontSRV;
};

