#include "FluxEngine.h"
#include "../Texture.h"
#include "../Graphics.h"

#include "D3D11GraphicsImpl.h"

#include "External/Stb/stb_image_write.h"

bool Texture::SetData(const void* pData)
{
	AUTOPROFILE(Texture_SetData);

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
		FLUX_LOG(Error, "[Texture::SetData()] > Not yet implemented!");
		return false;
	}
	return true;
}

void Texture::UpdateParameters()
{
	if ((m_pSamplerState && !m_ParametersDirty) || m_pResource == nullptr)
		return;

	AUTOPROFILE(Texture_CreateTextureSampler);

	SafeRelease(m_pSamplerState);

	D3D11_SAMPLER_DESC desc = {};
	Color borderColor = Color();
	TextureAddressMode mode = TextureAddressMode::WRAP;
	switch (mode)
	{
	case TextureAddressMode::WRAP:
		desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		break;
	case TextureAddressMode::MIRROR:
		desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
		break;
	case TextureAddressMode::CLAMP:
		desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		break;
	case TextureAddressMode::BORDER:
		desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		break;
	case TextureAddressMode::MIRROR_ONCE:
		desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		break;
	}

	memcpy(desc.BorderColor, &borderColor, 4 * sizeof(float));
	desc.ComparisonFunc = D3D11ComparisonFunction(CompareMode::LESSEQUAL);
	desc.Filter = D3D11Filter(TextureFilter::MIN_LINEAR_MAG_POINT_MIP_LINEAR);
	desc.MaxAnisotropy = 1;
	desc.MinLOD = std::numeric_limits<float>::min();
	desc.MaxLOD = std::numeric_limits<float>::max();

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateSamplerState(&desc, (ID3D11SamplerState**)&m_pSamplerState));
}

void Texture::UpdateProperties(void* pTexture)
{
	ID3D11Texture2D* pTex = static_cast<ID3D11Texture2D*>(pTexture);
	D3D11_TEXTURE2D_DESC desc;
	pTex->GetDesc(&desc);
	m_Width = desc.Width;
	m_Height = desc.Height;
}

bool Texture::Create()
{
	AUTOPROFILE(Texture_Create);

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