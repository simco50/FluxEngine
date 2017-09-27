#include "stdafx.h"
#include "Texture.h"
#include "Graphics.h"

Texture::Texture(ID3D11Resource* pTexture, ID3D11ShaderResourceView* pTextureSRV) :
	m_pResource(pTexture),
	m_pShaderResourceView(pTextureSRV),
	m_Usage(TextureUsage::STATIC)
{
	ID3D11Texture2D* pTex = static_cast<ID3D11Texture2D*>(pTexture);
	D3D11_TEXTURE2D_DESC desc;
	pTex->GetDesc(&desc);
	m_Width = desc.Width;
	m_Height = desc.Height;
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
}

bool Texture::Create()
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.ArraySize = 1;
	//desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (m_Usage == TextureUsage::DEPTHSTENCILBUFFER)
		desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
	else if (m_Usage == TextureUsage::RENDERTARGET)
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	else if (m_Usage == TextureUsage::DYNAMIC || m_Usage == TextureUsage::STATIC)
	{
	}
	else
	{
		FLUX_LOG(ERROR, "[Texture::Create()] > Usage type of texture not implemented or undefined!");
		return false;
	}
	desc.CPUAccessFlags = m_Usage == TextureUsage::DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.Format = (DXGI_FORMAT)m_TextureFormat;
	desc.Height = m_Height;
	desc.Width = m_Width;
	desc.MipLevels = 1; //#todo mip levels
	desc.MiscFlags = 0; //#todo D3D11_RESOURCE_MISC_GENERATE_MIPS
	desc.SampleDesc.Count = m_MultiSample;
	desc.SampleDesc.Quality = m_pGraphics->GetMultisampleQuality((DXGI_FORMAT)m_TextureFormat, m_MultiSample);
	desc.Usage = (m_Usage == TextureUsage::DYNAMIC) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	if (m_pResource == nullptr)
	{
		HR(m_pGraphics->GetDevice()->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)&m_pResource));
	}

	if ((desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) == D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = (m_MultiSample > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Format = (DXGI_FORMAT)GetSRVFormat(m_TextureFormat);

		HR(m_pGraphics->GetDevice()->CreateShaderResourceView((ID3D11Texture2D*)m_pResource, &srvDesc, (ID3D11ShaderResourceView**)&m_pShaderResourceView));
	}

	if ((desc.BindFlags & D3D11_BIND_RENDER_TARGET) == D3D11_BIND_RENDER_TARGET)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = desc.Format;
		rtvDesc.ViewDimension = (m_MultiSample > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;

		HR(m_pGraphics->GetDevice()->CreateRenderTargetView((ID3D11Texture2D*)m_pResource, nullptr /*&rtvDesc*/, (ID3D11RenderTargetView**)&m_pRenderTargetView));
	}

	else if ((desc.BindFlags & D3D11_BIND_DEPTH_STENCIL) == D3D11_BIND_DEPTH_STENCIL)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = (DXGI_FORMAT)GetDSVFormat(m_TextureFormat);
		dsvDesc.ViewDimension = (m_MultiSample > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

		HR(m_pGraphics->GetDevice()->CreateDepthStencilView((ID3D11Texture2D*)m_pResource, &dsvDesc, (ID3D11DepthStencilView**)&m_pRenderTargetView));
	}

	return true;
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
