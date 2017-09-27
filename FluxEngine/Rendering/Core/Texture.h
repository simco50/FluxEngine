#pragma once
class Graphics;

enum class TextureUsage
{
	STATIC,
	DYNAMIC,
	RENDERTARGET,
	DEPTHSTENCILBUFFER,
};

class Texture
{
public:
	Texture(Graphics* pGraphics);
	Texture(ID3D11Resource* pTexture, ID3D11ShaderResourceView* pTextureSRV);
	~Texture();

	DELETE_COPY(Texture)

	bool SetSize(const int width, const int height, const unsigned int format, TextureUsage usage, const int multiSample, void* pTexture);

	void* GetRenderTargetView() const { return m_pRenderTargetView; }
	void* GetResource() const { return m_pResource; }
	void* GetResourceView() const { return m_pShaderResourceView; }

	const int& GetWidth() const { return m_Width; }
	const int& GetHeight() const { return m_Height; }

private:
	void Release();
	bool Create();

	unsigned int GetSRVFormat(const unsigned int format);
	unsigned int GetDSVFormat(const unsigned int format);

	int m_Width = -1;
	int m_Height = -1;

	void* m_pResource = nullptr;
	void* m_pShaderResourceView = nullptr;
	void* m_pRenderTargetView = nullptr;
	void* m_pReadOnlyView = nullptr;

	unsigned int m_TextureFormat = 0;
	int m_MultiSample = 1;

	TextureUsage m_Usage = TextureUsage::STATIC;
	Graphics* m_pGraphics = nullptr;
};

