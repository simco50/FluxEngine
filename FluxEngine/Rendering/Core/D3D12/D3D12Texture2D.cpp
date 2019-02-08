#include "FluxEngine.h"
#include "Rendering/Core/Texture2D.h"
#include "Rendering/Core/Graphics.h"
#include "D3D12GraphicsImpl.h"
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
	m_pResource = (ID3D12Resource*)pTexture;

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
	return false;
}

bool Texture2D::Create()
{
	AUTOPROFILE(Texture2D_Create);

	if(m_pGraphics->GetImpl()->CheckMultisampleQuality((DXGI_FORMAT)m_TextureFormat, m_MultiSample))
	{
		FLUX_LOG(Info, "[Texture2D::Create()] Device does not support MSAA x%d. Disabling MSAA for this texture", m_MultiSample);
		m_MultiSample = 1;
	}

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = m_Width;
	desc.Height = m_Height;
	desc.DepthOrArraySize = 1;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	if (m_Usage == TextureUsage::RENDERTARGET)
	{
		desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	else if (m_Usage == TextureUsage::DEPTHSTENCILBUFFER)
	{
		desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	desc.Format = (DXGI_FORMAT)m_TextureFormat;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.MipLevels = m_MipLevels;
	desc.SampleDesc.Count = m_MultiSample;
	desc.SampleDesc.Quality = m_pGraphics->GetImpl()->GetMultisampleQuality((DXGI_FORMAT)m_TextureFormat, m_MultiSample);

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = (DXGI_FORMAT)m_TextureFormat;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	if (m_pResource == nullptr)
	{
		HR(m_pGraphics->GetImpl()->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			m_Usage == TextureUsage::DEPTHSTENCILBUFFER ? &clearValue : nullptr,
			IID_PPV_ARGS((ID3D12Resource**)&m_pResource)));
	}

	D3D12_CPU_DESCRIPTOR_HANDLE h;
	if (1 /*Shader Resource*/)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = m_MipLevels;
		srvDesc.Format = (DXGI_FORMAT)GetSRVFormat(m_TextureFormat);

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(h);
		m_pGraphics->GetImpl()->GetDevice()->CreateShaderResourceView(
			m_pResolvedResource ? (ID3D12Resource*)m_pResolvedResource : (ID3D12Resource*)m_pResource,
			&srvDesc,
			handle);
	}

	if (m_Usage == TextureUsage::RENDERTARGET)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = (DXGI_FORMAT)m_TextureFormat;
		rtvDesc.ViewDimension = m_MultiSample > 1 ? D3D12_RTV_DIMENSION_TEXTURE2DMS : D3D12_RTV_DIMENSION_TEXTURE2D;

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(h);
		m_pGraphics->GetImpl()->GetDevice()->CreateRenderTargetView((ID3D12Resource*)m_pResource,
			&rtvDesc,
			handle);
	}

	else if (m_Usage == TextureUsage::DEPTHSTENCILBUFFER)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = (DXGI_FORMAT)GetDSVFormat(m_TextureFormat);
		dsvDesc.ViewDimension = (m_MultiSample > 1) ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(h);
		m_pGraphics->GetImpl()->GetDevice()->CreateDepthStencilView((ID3D12Resource*)m_pResource, 
			&dsvDesc, 
			handle);
	}

	return true;
}

bool Texture2D::Resolve(bool force)
{
	return false;
}