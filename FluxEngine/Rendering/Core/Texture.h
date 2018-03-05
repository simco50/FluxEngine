#pragma once
#include "Content/Resource.h"

class Graphics;
class Image;

enum class TextureUsage
{
	STATIC,
	DYNAMIC,
	RENDERTARGET,
	DEPTHSTENCILBUFFER,
};

class Texture : public Resource
{
	FLUX_OBJECT(Texture, Resource)

public:
	Texture(Context* pContext);
	Texture(Context* pContext, void* pTexture, void* pTextureSRV);
	~Texture();

	DELETE_COPY(Texture)

	virtual bool Load(InputStream& inputStream) override;
	virtual bool Save(OutputStream& outputStream) override;

	bool SetSize(const int width, const int height, const unsigned int format, TextureUsage usage, const int multiSample, void* pTexture);
	bool SetData(const void* pData);

	void* GetRenderTargetView() const { return m_pRenderTargetView; }
	void* GetResource() const { return m_pResource; }
	void* GetResourceView() const { return m_pShaderResourceView; }
	void* GetSamplerState() const { return m_pSamplerState; }

	const int& GetWidth() const { return m_Width; }
	const int& GetHeight() const { return m_Height; }

	void UpdateParameters();

private:
	void UpdateProperties(void* pTexture);

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
	bool m_ParametersDirty = false;

	unsigned int m_TextureFormat = 0;
	unsigned int m_MultiSample = 1;

	std::unique_ptr<Image> m_pImage;
	TextureUsage m_Usage = TextureUsage::STATIC;
	Graphics* m_pGraphics = nullptr;
};

