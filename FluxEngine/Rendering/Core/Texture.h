#pragma once
class Graphics;

enum class TextureUsage
{
	STATIC,
	DYNAMIC,
	RENDERTARGET,
	DEPTHSTENCILBUFFER,
};

enum class TextureAddressMode
{
	WRAP,
	MIRROR,
	CLAMP,
	BORDER,
	MIRROR_ONCE,
};

enum class TextureFilter
{
	MIN_MAG_MIP_POINT,
	MIN_MAG_POINT_MIP_LINEAR,
	MIN_POINT_MAG_LINEAR_MIP_POINT,
	MIN_POINT_MAG_MIP_LINEAR,
	MIN_LINEAR_MAG_MIP_POINT,
	MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	MIN_MAG_LINEAR_MIP_POINT,
	MIN_MAG_MIP_LINEAR,
	ANISOTROPIC,
	COMPARISON_MIN_MAG_MIP_POINT,
	COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
	COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
	COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
	COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
	COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
	COMPARISON_MIN_MAG_MIP_LINEAR,
	COMPARISON_ANISOTROPIC,
	MINIMUM_MIN_MAG_MIP_POINT,
	MINIMUM_MIN_MAG_POINT_MIP_LINEAR,
	MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
	MINIMUM_MIN_POINT_MAG_MIP_LINEAR,
	MINIMUM_MIN_LINEAR_MAG_MIP_POINT,
	MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	MINIMUM_MIN_MAG_LINEAR_MIP_POINT,
	MINIMUM_MIN_MAG_MIP_LINEAR,
	MINIMUM_ANISOTROPIC,
	MAXIMUM_MIN_MAG_MIP_POINT,
	MAXIMUM_MIN_MAG_POINT_MIP_LINEAR,
	MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
	MAXIMUM_MIN_POINT_MAG_MIP_LINEAR,
	MAXIMUM_MIN_LINEAR_MAG_MIP_POINT,
	MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	MAXIMUM_MIN_MAG_LINEAR_MIP_POINT,
	MAXIMUM_MIN_MAG_MIP_LINEAR,
	MAXIMUM_ANISOTROPIC,
};

class Texture
{
public:
	Texture(Graphics* pGraphics);
	Texture(Graphics* pGraphics, ID3D11Resource* pTexture, ID3D11ShaderResourceView* pTextureSRV);
	~Texture();

	DELETE_COPY(Texture)

	bool SetSize(const int width, const int height, const unsigned int format, TextureUsage usage, const int multiSample, void* pTexture);

	void* GetRenderTargetView() const { return m_pRenderTargetView; }
	void* GetResource() const { return m_pResource; }
	void* GetResourceView() const { return m_pShaderResourceView; }

	void* GetSamplerState() const { return m_pSamplerState; }

	const int& GetWidth() const { return m_Width; }
	const int& GetHeight() const { return m_Height; }

	void UpdateParameters();

private:
	void Release();
	bool Create();

	unsigned int GetSRVFormat(const unsigned int format);
	unsigned int GetDSVFormat(const unsigned int format);

	unsigned int m_Width = 0;
	unsigned int m_Height = 0;

	void* m_pResource = nullptr;
	void* m_pShaderResourceView = nullptr;
	void* m_pRenderTargetView = nullptr;
	void* m_pReadOnlyView = nullptr;

	void* m_pSamplerState = nullptr;

	unsigned int m_TextureFormat = 0;
	unsigned int m_MultiSample = 1;

	TextureUsage m_Usage = TextureUsage::STATIC;
	Graphics* m_pGraphics = nullptr;
};

