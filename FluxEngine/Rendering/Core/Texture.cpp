#include "FluxEngine.h"
#include "Texture.h"
#include "Graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include "External/Stb/stb_image.h"

/*namespace STBI
{
	int ReadCallback(void* pUser, char* pData, int size)
	{
		InputStream* pStream = (InputStream*)pUser;
		if (pStream == nullptr)
			return 0;
		return (int)pStream->Read(pData, (size_t)size);
	}

	void SkipCallback(void* pUser, int n)
	{
		InputStream* pStream = (InputStream*)pUser;
		if (pStream)
			pStream->MovePointer(n);
	}

	int EofCallback(void* pUser)
	{
		InputStream* pStream = (InputStream*)pUser;
		if (pStream == nullptr)
			return 1;
		return pStream->GetPointer() >= pStream->GetSize() ? 1 : 0;
	}
}*/

Texture::Texture(Context* pContext, void* pTexture, void* pTextureSRV) :
	Resource(pContext),
	m_pResource(pTexture),
	m_pShaderResourceView(pTextureSRV),
	m_Usage(TextureUsage::STATIC)
{
	m_pGraphics = pContext->GetSubsystem<Graphics>();
	UpdateProperties(pTexture);
}

Texture::Texture(Context* pContext) :
	Resource(pContext)
{
	m_pGraphics = pContext->GetSubsystem<Graphics>();
}

Texture::~Texture()
{
	Release();
}

bool Texture::Load(InputStream& inputStream)
{
	AUTOPROFILE(Texture_Load);

	unsigned char* pPixels = nullptr;
	int width, height, bpp;
	std::vector<unsigned char> buffer;
	inputStream.ReadAllBytes(buffer);
	pPixels = stbi_load_from_memory(buffer.data(), buffer.size(), &width, &height, &bpp, 4);
	/*{
		AUTOPROFILE(Texture_Load_FromMemory);
		stbi_io_callbacks callbacks;
		callbacks.read = STBI::ReadCallback;
		callbacks.skip = STBI::SkipCallback;
		callbacks.eof = STBI::EofCallback;
		pPixels = stbi_load_from_callbacks(&callbacks, &inputStream, &width, &height, &bpp, 4);
		if (pPixels == nullptr)
			return false;
	}*/

	if (!SetSize(width, height, DXGI_FORMAT_R8G8B8A8_UNORM, TextureUsage::STATIC, 1, nullptr))
		return false;
	if (!SetData(pPixels))
		return false;

	stbi_image_free(pPixels);
	return true;
}

bool Texture::SetSize(const int width, const int height, const unsigned int format, TextureUsage usage, const int multiSample, void* pTexture)
{
	AUTOPROFILE(Texture_SetSize);

	if (multiSample > 1 && usage != TextureUsage::DEPTHSTENCILBUFFER && usage != TextureUsage::RENDERTARGET)
	{
		FLUX_LOG(Error, "[Texture::SetSize()] > Multisampling is only supported for rendertarget or depth-stencil textures");
		return false;
	}

	Release();

	m_Width = width;
	m_Height = height;
	m_TextureFormat = format;
	m_Usage = usage;
	m_MultiSample = multiSample;
	m_pResource = pTexture;

	if (!Create())
		return false;
	return true;
}

void Texture::Release()
{
	SafeRelease(m_pResource);
	SafeRelease(m_pShaderResourceView);
	SafeRelease(m_pRenderTargetView);
	SafeRelease(m_pReadOnlyView);
	SafeRelease(m_pSamplerState);
}


unsigned int Texture::GetSRVFormat(const unsigned int format)
{
	if (format == DXGI_FORMAT_R24G8_TYPELESS)
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	else if (format == DXGI_FORMAT_R16_TYPELESS)
		return DXGI_FORMAT_R16_UNORM;
	else if (format == DXGI_FORMAT_R32_TYPELESS)
		return DXGI_FORMAT_R32_FLOAT;
	else
		return format;
}

unsigned int Texture::GetDSVFormat(const unsigned int format)
{
	if (format == DXGI_FORMAT_R24G8_TYPELESS)
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	else if (format == DXGI_FORMAT_R16_TYPELESS)
		return DXGI_FORMAT_D16_UNORM;
	else if (format == DXGI_FORMAT_R32_TYPELESS)
		return DXGI_FORMAT_D32_FLOAT;
	else
		return format;
}