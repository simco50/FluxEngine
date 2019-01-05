#include "FluxEngine.h"
#include "Rendering/Core/Texture2D.h"
#include "Rendering/Core/Graphics.h"
#include "D3D11GraphicsImpl.h"
#include "Content/Image.h"
#include "Rendering/Core/RenderTarget.h"

Texture2D::Texture2D(Context* pContext)
	: Texture(pContext)
{
}

Texture2D::~Texture2D()
{
	Texture2D::Release();
}

void Texture2D::Release()
{
	SafeRelease(m_pResource);
	SafeRelease(m_pShaderResourceView);
	SafeRelease(m_pSamplerState);
	SafeRelease(m_pResolvedResource);

	m_pRenderTarget.reset();
}

bool Texture2D::Load(InputStream& inputStream)
{
	AUTOPROFILE(Texture2D_Load);

	m_pImage = std::make_unique<Image>(m_pContext);
	if (m_pImage->Load(inputStream) == false)
	{
		return false;
	}

	if (SetImage(*m_pImage) == false)
	{
		return false;
	}

	return true;
}

bool Texture2D::SetImage(const Image& image)
{
	AUTOPROFILE_DESC(Texture2D_SetImage, image.GetFilePath().c_str());

	uint32 memoryUsage = 0;

	ImageFormat compressionFormat = image.GetFormat();
	m_MipLevels = image.GetMipLevels();

	if (!SetSize(image.GetWidth(), image.GetHeight(), TextureFormatFromCompressionFormat(compressionFormat, image.IsSRGB()), TextureUsage::STATIC, 1, nullptr))
	{
		return false;
	}

	AUTOPROFILE_DESC(Texture2D_SetImage_Compressed, "Compressed load");
	for (int mipLevel = 0; mipLevel < image.GetMipLevels(); ++mipLevel)
	{
		MipLevelInfo mipData = image.GetMipInfo(mipLevel);
		if (!SetData(mipLevel, 0, 0, mipData.Width, mipData.Height, image.GetData(mipLevel)))
		{
			return false;
		}
		memoryUsage += mipData.DataSize;
	}

	SetMemoryUsage(memoryUsage);

	return true;
}

bool Texture2D::SetSize(int width, int height, unsigned int format, TextureUsage usage, int multiSample, void* pTexture)
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
	m_pResource = (ID3D11Resource*)pTexture;

	if (usage == TextureUsage::RENDERTARGET || usage == TextureUsage::DEPTHSTENCILBUFFER)
	{
		m_pRenderTarget = std::make_unique<RenderTarget>(this);
	}

	if (!Create())
	{
		return false;
	}
	return true;
}

bool Texture2D::SetData(unsigned int mipLevel, int x, int y, int width, int height, const void* pData)
{
	AUTOPROFILE(Texture2D_SetData);

	int levelWidth = GetLevelWidth(mipLevel);
	int levelHeight = GetLevelHeight(mipLevel);
	if (x < 0 || x + width > levelWidth || y < 0 || y + height > levelHeight || width <= 0 || height <= 0)
	{
		FLUX_LOG(Warning, "[Texture2D::SetData] Illegal dimensions for setting data");
		return false;
	}

	if (IsCompressed())
	{
		x &= ~3;
		y &= ~3;
		width += 3;
		width &= 0xfffffffc;
		height += 3;
		height &= 0xfffffffc;
	}

	unsigned int rowSize = GetRowDataSize(width);
	unsigned int rowStart = GetRowDataSize(x);
	unsigned int subResource = D3D11CalcSubresource(mipLevel, 0, m_MipLevels);

	if (m_Usage == TextureUsage::STATIC)
	{
		D3D11_BOX box;
		box.left = (UINT)x;
		box.right = (UINT)(x + width);
		box.top = (UINT)y;
		box.bottom = (UINT)(y + height);
		box.front = 0;
		box.back = 1;
		m_pGraphics->GetImpl()->GetDeviceContext()->UpdateSubresource((ID3D11Buffer*)m_pResource, subResource, &box, pData, rowSize, 0);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE mapData = {};
		HR(m_pGraphics->GetImpl()->GetDeviceContext()->Map((ID3D11Buffer*)m_pResource, subResource, D3D11_MAP_WRITE_DISCARD, 0, &mapData));
		for (int rowIdx = 0; rowIdx < height; ++rowIdx)
		{
			memcpy((char*)mapData.pData + (rowIdx + y) * mapData.RowPitch + rowStart, (char*)pData + rowIdx * rowSize, rowSize);
		}
		m_pGraphics->GetImpl()->GetDeviceContext()->Unmap((ID3D11Buffer*)m_pResource, subResource);
	}

	return true;
}

bool Texture2D::Create()
{
	AUTOPROFILE(Texture2D_Create);

	D3D11_TEXTURE2D_DESC desc = {};

	if (m_pGraphics->GetImpl()->CheckMultisampleQuality((DXGI_FORMAT)m_TextureFormat, m_MultiSample) == false)
	{
		FLUX_LOG(Info, "[Texture2D::Create()] Device does not support MSAA x%d. Disabling MSAA for this texture", m_MultiSample);
		m_MultiSample = 1;
	}

	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (m_Usage == TextureUsage::DEPTHSTENCILBUFFER)
	{
		desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		m_MipLevels = 1;
	}
	else if (m_Usage == TextureUsage::RENDERTARGET)
	{
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		if (m_MipLevels != 1 && m_MultiSample == 1)
		{
			desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
	}
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
	desc.MipLevels = m_MipLevels;
	desc.SampleDesc.Count = m_MultiSample;
	desc.SampleDesc.Quality = m_pGraphics->GetImpl()->GetMultisampleQuality((DXGI_FORMAT)m_TextureFormat, m_MultiSample);
	desc.Usage = (m_Usage == TextureUsage::DYNAMIC) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	if (m_pResource == nullptr)
	{
		HR(m_pGraphics->GetImpl()->GetDevice()->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)&m_pResource));
	}

	//Create resolve texture if requested
	if (m_MultiSample > 1)
	{
		desc.MipLevels = m_MipLevels;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		if (m_MipLevels != 1)
		{
			desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		HR(m_pGraphics->GetImpl()->GetDevice()->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)&m_pResolvedResource));
	}

	if ((desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) == D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = m_MipLevels;
		srvDesc.Format = (DXGI_FORMAT)GetSRVFormat(m_TextureFormat);

		HR(m_pGraphics->GetImpl()->GetDevice()->CreateShaderResourceView(m_pResolvedResource ? (ID3D11Texture2D*)m_pResolvedResource : (ID3D11Texture2D*)m_pResource, &srvDesc, (ID3D11ShaderResourceView**)&m_pShaderResourceView));
	}

	if ((desc.BindFlags & D3D11_BIND_RENDER_TARGET) == D3D11_BIND_RENDER_TARGET)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = (DXGI_FORMAT)m_TextureFormat;
		rtvDesc.ViewDimension = m_MultiSample > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
		HR(m_pGraphics->GetImpl()->GetDevice()->CreateRenderTargetView((ID3D11Texture2D*)m_pResource, &rtvDesc, (ID3D11RenderTargetView**)&m_pRenderTarget->m_pRenderTargetView));
	}

	else if ((desc.BindFlags & D3D11_BIND_DEPTH_STENCIL) == D3D11_BIND_DEPTH_STENCIL)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = (DXGI_FORMAT)GetDSVFormat(m_TextureFormat);
		dsvDesc.ViewDimension = (m_MultiSample > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

		HR(m_pGraphics->GetImpl()->GetDevice()->CreateDepthStencilView((ID3D11Texture2D*)m_pResource, &dsvDesc, (ID3D11DepthStencilView**)&m_pRenderTarget->m_pRenderTargetView));
	}
	return true;
}

bool Texture2D::Resolve(bool force)
{
	if (m_ResolveTextureDirty || force)
	{
		if (m_pResolvedResource == nullptr)
		{
			return false;
		}
		m_pGraphics->GetImpl()->GetDeviceContext()->ResolveSubresource((ID3D11Texture2D*)m_pResolvedResource, 0, (ID3D11Texture2D*)m_pResource, 0, (DXGI_FORMAT)m_TextureFormat);
		m_ResolveTextureDirty = false;
	}
	return true;
}