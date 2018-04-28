#include "FluxEngine.h"
#include "../Texture2D.h"
#include "../Graphics.h"

#include "D3D11GraphicsImpl.h"
#include "Content/Image.h"

bool Texture2D::Load(InputStream& inputStream)
{
	AUTOPROFILE(Texture2D_Load);

	m_pImage = std::make_unique<Image>(m_pContext);
	if (!m_pImage->Load(inputStream))
		return false;
	//m_pImage->ConvertToRGBA();
	if (!SetSize(m_pImage->GetWidth(), m_pImage->GetHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, TextureUsage::STATIC, 1, nullptr))
		return false;
	if (!SetData(m_pImage->GetData()))
		return false;

	return true;
}

bool Texture2D::SetSize(const int width, const int height, const unsigned int format, TextureUsage usage, const int multiSample, void* pTexture)
{
	AUTOPROFILE(Texture2D_SetSize);

	if (multiSample > 1 && usage != TextureUsage::DEPTHSTENCILBUFFER && usage != TextureUsage::RENDERTARGET)
	{
		FLUX_LOG(Error, "[Texture::SetSize()] > Multisampling is only supported for rendertarget or depth-stencil textures");
		return false;
	}

	Release();

	m_Width = width;
	m_Height = height;
	m_Depth = 1;
	m_TextureFormat = format;
	m_Usage = usage;
	m_MultiSample = multiSample;
	m_pResource = pTexture;

	if (!Create())
		return false;
	return true;
}

bool Texture2D::SetData(const void* pData)
{
	AUTOPROFILE(Texture2D_SetData);

	if (m_Usage == TextureUsage::STATIC)
	{
		D3D11_BOX box;
		box.back = 1;
		box.front = 0;
		box.left = 0;
		box.top = 0;
		box.right = m_Width;
		box.bottom = m_Height;
		m_pGraphics->GetImpl()->GetDeviceContext()->UpdateSubresource((ID3D11Buffer*)m_pResource, 0, &box, pData, m_Width * 4, 0);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE subResource = {};
		HR(m_pGraphics->GetImpl()->GetDeviceContext()->Map((ID3D11Buffer*)m_pResource, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource));
		void* pTarget = subResource.pData;
		memcpy(pTarget, pData, m_Width * m_Height * 4);
		m_pGraphics->GetImpl()->GetDeviceContext()->Unmap((ID3D11Buffer*)m_pResource, 0);
	}

	SetMemoryUsage(m_Height * m_Width * 4);

	return true;
}

bool Texture2D::Create()
{
	AUTOPROFILE(Texture2D_Create);

	D3D11_TEXTURE2D_DESC desc = {};
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (m_Usage == TextureUsage::DEPTHSTENCILBUFFER)
		desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
	else if (m_Usage == TextureUsage::RENDERTARGET)
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	else if (m_Usage == TextureUsage::DYNAMIC || m_Usage == TextureUsage::STATIC)
	{
	}
	else
	{
		FLUX_LOG(Error, "[Texture::Create()] > Usage type of texture not implemented or undefined!");
		return false;
	}
	desc.CPUAccessFlags = m_Usage == TextureUsage::DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.Format = (DXGI_FORMAT)m_TextureFormat;
	desc.Height = m_Height;
	desc.Width = m_Width;
	desc.MipLevels = 1; //#todo mip levels
	desc.MiscFlags = 0; //#todo D3D11_RESOURCE_MISC_GENERATE_MIPS
	desc.SampleDesc.Count = m_MultiSample;
	desc.SampleDesc.Quality = m_pGraphics->GetImpl()->GetMultisampleQuality((DXGI_FORMAT)m_TextureFormat, m_MultiSample);
	desc.Usage = (m_Usage == TextureUsage::DYNAMIC) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	if (m_pResource == nullptr)
	{
		HR(m_pGraphics->GetImpl()->GetDevice()->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)&m_pResource));
	}

	if ((desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) == D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = (m_MultiSample > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Format = (DXGI_FORMAT)GetSRVFormat(m_TextureFormat);

		HR(m_pGraphics->GetImpl()->GetDevice()->CreateShaderResourceView((ID3D11Texture2D*)m_pResource, &srvDesc, (ID3D11ShaderResourceView**)&m_pShaderResourceView));
	}

	if ((desc.BindFlags & D3D11_BIND_RENDER_TARGET) == D3D11_BIND_RENDER_TARGET)
	{
		HR(m_pGraphics->GetImpl()->GetDevice()->CreateRenderTargetView((ID3D11Texture2D*)m_pResource, nullptr, (ID3D11RenderTargetView**)&m_pRenderTargetView));
	}

	else if ((desc.BindFlags & D3D11_BIND_DEPTH_STENCIL) == D3D11_BIND_DEPTH_STENCIL)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = (DXGI_FORMAT)GetDSVFormat(m_TextureFormat);
		dsvDesc.ViewDimension = (m_MultiSample > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

		HR(m_pGraphics->GetImpl()->GetDevice()->CreateDepthStencilView((ID3D11Texture2D*)m_pResource, &dsvDesc, (ID3D11DepthStencilView**)&m_pRenderTargetView));
	}

	return true;
}
