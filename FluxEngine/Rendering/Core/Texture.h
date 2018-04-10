#pragma once
#include "Content/Resource.h"
#include "GraphicsDefines.h"

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
	virtual ~Texture();

	DELETE_COPY(Texture)

	virtual bool Load(InputStream& inputStream) override { UNREFERENCED_PARAMETER(inputStream); return true; }

	void SetAddressMode(const TextureAddressMode addressMode);

	void* GetRenderTargetView() const { return m_pRenderTargetView; }
	void* GetResource() const { return m_pResource; }
	void* GetResourceView() const { return m_pShaderResourceView; }
	void* GetSamplerState() const { return m_pSamplerState; }

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	int GetDepth() const { return m_Depth; }

	void UpdateParameters();

protected:
	void Release();
	virtual bool Create() { return true; }

	unsigned int GetSRVFormat(const unsigned int format);
	unsigned int GetDSVFormat(const unsigned int format);

	unsigned int m_Width = 0;
	unsigned int m_Height = 0;
	unsigned int m_Depth = 0;

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
	TextureAddressMode m_AddressMode = TextureAddressMode::WRAP;
	Graphics* m_pGraphics = nullptr;
};

