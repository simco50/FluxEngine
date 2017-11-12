#include "D3D11GraphicsImpl.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "External/Stb/stb_image_write.h"
#include "FileSystem/File/PhysicalFile.h"

bool Texture::SetData(void* pData)
{
	if (m_Usage == TextureUsage::STATIC)
	{
		D3D11_BOX box = {};
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
		FLUX_LOG(ERROR, "[Texture::SetData()] > Not yet implemented!");
		return false;
	}
	return true;
}

bool Texture::Save(const std::string& filePath)
{
	std::string extension = Paths::GetFileExtenstion(filePath);

	if (extension != "png")
	{
		FLUX_LOG(ERROR, "[Texture::Save] > Only .png is supported");
		return false;
	}

	AUTOPROFILE(Texture_Save);

	D3D11_TEXTURE2D_DESC desc = {};
	desc.ArraySize = 1;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Width = m_Width;
	desc.Height = m_Height;
	desc.MipLevels = 1;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_STAGING;

	ComPtr<ID3D11Texture2D> pStagingTexture;
	HR(m_pGraphics->GetImpl()->GetDevice()->CreateTexture2D(&desc, nullptr, pStagingTexture.GetAddressOf()));

	//If we are using MSAA, we need to resolve the resource first
	if (m_MultiSample > 1)
	{
		ComPtr<ID3D11Texture2D> pResolveTexture;

		D3D11_TEXTURE2D_DESC resolveTexDesc = {};
		resolveTexDesc.ArraySize = 1;
		resolveTexDesc.CPUAccessFlags = 0;
		resolveTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		resolveTexDesc.Width = m_Width;
		resolveTexDesc.Height = m_Height;
		resolveTexDesc.MipLevels = 1;
		resolveTexDesc.MiscFlags = 0;
		resolveTexDesc.SampleDesc.Count = 1;
		resolveTexDesc.SampleDesc.Quality = 0;
		resolveTexDesc.Usage = D3D11_USAGE_DEFAULT;

		HR(m_pGraphics->GetImpl()->GetDevice()->CreateTexture2D(&resolveTexDesc, nullptr, pResolveTexture.GetAddressOf()));

		m_pGraphics->GetImpl()->GetDeviceContext()->ResolveSubresource(pResolveTexture.Get(), 0, (ID3D11Texture2D*)m_pResource, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		m_pGraphics->GetImpl()->GetDeviceContext()->CopyResource(pStagingTexture.Get(), pResolveTexture.Get());
	}
	else
	{
		m_pGraphics->GetImpl()->GetDeviceContext()->CopyResource(pStagingTexture.Get(), (ID3D11Texture2D*)m_pResource);
	}

	D3D11_MAPPED_SUBRESOURCE pData = {};
	m_pGraphics->GetImpl()->GetDeviceContext()->Map(pStagingTexture.Get(), 0, D3D11_MAP_READ, 0, &pData);

	stbi_write_png_to_func([](void *context, void *data, int size)
	{
		UNREFERENCED_PARAMETER(context);

		stringstream str;
		str << Paths::ScreenshotFolder << "\\" << GetTimeStamp() << ".png";
		PhysicalFile pFile(str.str());
		if (!pFile.Open(FileMode::Write, ContentType::Binary))
			return;
		if (!pFile.Write((char*)data, size))
			return;
		pFile.Close();

	}, nullptr, m_Width, m_Height, 4, pData.pData, pData.RowPitch);
	m_pGraphics->GetImpl()->GetDeviceContext()->Unmap(pStagingTexture.Get(), 0);
	return true;
}

void Texture::UpdateParameters()
{
	if ((m_pSamplerState && !m_ParametersDirty) || m_pResource == nullptr)
		return;

	AUTOPROFILE(CreateTextureSampler);

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
	desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	desc.MaxAnisotropy = 1;
	desc.MinLOD = numeric_limits<float>::min();
	desc.MaxLOD = numeric_limits<float>::max();

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
	D3D11_TEXTURE2D_DESC desc = {};
	desc.ArraySize = 1;
	//#todo Create a SRV from a RT and DB
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (m_Usage == TextureUsage::DEPTHSTENCILBUFFER)
		desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
	else if (m_Usage == TextureUsage::RENDERTARGET)
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	else if (m_Usage == TextureUsage::DYNAMIC || m_Usage == TextureUsage::STATIC)
	{
		//desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
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
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = desc.Format;
		rtvDesc.ViewDimension = (m_MultiSample > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;

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