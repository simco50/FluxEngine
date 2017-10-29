#include "stdafx.h"
#include "Texture.h"
#include "Graphics.h"

#ifdef D3D11
#include "D3D11/D3D11Texture.hpp"
#endif

Texture::Texture(Graphics* pGraphics, void* pTexture, void* pTextureSRV) :
	m_pGraphics(pGraphics),
	m_pResource(pTexture),
	m_pShaderResourceView(pTextureSRV),
	m_Usage(TextureUsage::STATIC)
{
	UpdateProperties(pTexture);
}

Texture::Texture(Graphics* pGraphics) :
	m_pGraphics(pGraphics)
{

}

Texture::~Texture()
{
	Release();
}

bool Texture::SetSize(const int width, const int height, const unsigned int format, TextureUsage usage, const int multiSample, void* pTexture)
{
	if (multiSample > 1 && usage != TextureUsage::DEPTHSTENCILBUFFER && usage != TextureUsage::RENDERTARGET)
	{
		FLUX_LOG(ERROR, "[Texture::SetSize()] > Multisampling is only supported for rendertarget or depth-stencil textures");
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