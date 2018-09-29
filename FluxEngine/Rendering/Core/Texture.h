#pragma once
#include "Content/Resource.h"
#include "GraphicsDefines.h"

class Graphics;
class Image;

enum class ImageFormat;

enum class TextureUsage
{
	STATIC,
	DYNAMIC,
	RENDERTARGET,
	DEPTHSTENCILBUFFER,
	MAX
};

enum class TextureAddressMode
{
	WRAP = 0,
	MIRROR,
	CLAMP,
	BORDER,
	MIRROR_ONCE,
	MAX
};

class Texture : public Resource
{
	FLUX_OBJECT(Texture, Resource)

public:
	Texture(Context* pContext);
	virtual ~Texture();

	DELETE_COPY(Texture)

	virtual void Release() = 0;

	virtual bool Load(InputStream& /*inputStream*/) override { return true; }
	virtual bool Resolve(bool force) = 0;

	void SetAddressMode(TextureAddressMode addressMode);
	void SetMipLevels(const int mipLevels) { m_MipLevels = mipLevels; }
	void SetResolveDirty(bool dirty) { m_ResolveTextureDirty = dirty; }
	void RegenerateMips();
	void UpdateParameters();

	void* GetResource() const { return m_pResource; }
	void* GetResolvedResource() const { return m_pResolvedResource; }
	void* GetResourceView() const { return m_pShaderResourceView; }
	void* GetSamplerState() const { return m_pSamplerState; }

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	int GetDepth() const { return m_Depth; }
	int GetMultiSample() const { return m_MultiSample; }
	bool GetResolveDirty() const { return m_ResolveTextureDirty; }
	unsigned int GetFormat() const { return m_TextureFormat; }
	bool IsCompressed() const;

	const Image* GetImage() const { return m_pImage.get(); }

protected:
	virtual bool Create() { return true; }

	int GetLevelWidth(unsigned int mipLevel) const;
	int GetLevelHeight(unsigned int mipLevel) const;
	int GetLevelDepth(unsigned int mipLevel) const;
	int GetRowDataSize(unsigned int width) const;

	static unsigned int GetSRVFormat(unsigned int format);
	static unsigned int GetDSVFormat(unsigned int format);

	static unsigned int TextureFormatFromCompressionFormat(const ImageFormat& format, bool sRgb);

	unsigned int m_Width = 0;
	unsigned int m_Height = 0;
	unsigned int m_Depth = 0;
	unsigned int m_MipLevels = 1;

	void* m_pResource = nullptr;
	void* m_pResolvedResource = nullptr;
	void* m_pShaderResourceView = nullptr;

	void* m_pSamplerState = nullptr;
	bool m_ParametersDirty = false;
	bool m_ResolveTextureDirty = false;

	unsigned int m_TextureFormat = 0;
	unsigned int m_MultiSample = 1;

	std::unique_ptr<Image> m_pImage;
	TextureUsage m_Usage = TextureUsage::STATIC;
	TextureAddressMode m_AddressMode = TextureAddressMode::WRAP;
	Graphics* m_pGraphics = nullptr;
};

