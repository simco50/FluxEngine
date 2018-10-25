#include "FluxEngine.h"
#include "../Texture3D.h"
#include "../Graphics.h"

#include "D3D11GraphicsImpl.h"
#include "Content/Image.h"

Texture3D::Texture3D(Context* pContext)
	: Texture(pContext)
{

}

Texture3D::~Texture3D()
{
	Texture3D::Release();
}

void Texture3D::Release()
{
	SafeRelease(m_pResource);
	SafeRelease(m_pShaderResourceView);
	SafeRelease(m_pSamplerState);
	SafeRelease(m_pResolvedResource);
}

bool Texture3D::Load(InputStream& inputStream)
{
	AUTOPROFILE(Texture3D_Load);

	m_pImage = std::make_unique<Image>(m_pContext);
	if (m_pImage->Load(inputStream) == false)
	{
		return false;
	}
	if (SetImage(*m_pImage) == false)
	{
		return false;
	}

	GRAPHICS_SET_NAME(inputStream.GetSource());

	return true;
}

bool Texture3D::SetSize(int width, int height, int depth, unsigned int format, TextureUsage usage, int multiSample, void* pTexture)
{
	AUTOPROFILE(Texture3D_SetSize);

	if (multiSample > 1 && usage != TextureUsage::DEPTHSTENCILBUFFER && usage != TextureUsage::RENDERTARGET)
	{
		FLUX_LOG(Error, "[Texture::SetSize()] > Multisampling is only supported for rendertarget or depth-stencil textures");
		return false;
	}

	Release();

	m_Width = width;
	m_Height = height;
	m_Depth = depth;
	m_TextureFormat = format;
	m_Usage = usage;
	m_MultiSample = multiSample;
	m_pResource = (ID3D11Resource*)pTexture;

	if (!Create())
		return false;
	return true;
}

bool Texture3D::SetData(unsigned int mipLevel, int x, int y, int z, int width, int height, int depth, const void* pData)
{
	AUTOPROFILE(Texture3D_SetData);

	int32 levelWidth = GetLevelWidth(mipLevel);
	int32 levelHeight = GetLevelHeight(mipLevel);
	int32 levelDepth = GetLevelDepth(mipLevel);

	if (x < 0 || x + width > levelWidth || y < 0 || y + height > levelHeight || z < 0 || z + depth > levelDepth || width <= 0 ||
		height <= 0 || depth <= 0)
	{
		FLUX_LOG(Error, "[Texture3D::SetData] Illegal dimensions for setting data");
		return false;
	}

	// If compressed, align the update region on a block
	if (IsCompressed())
	{
		x &= ~3;
		y &= ~3;
		width += 3;
		width &= 0xfffffffc;
		height += 3;
		height &= 0xfffffffc;
	}

	uint32 rowSize = GetRowDataSize(width);
	uint32 rowStart = GetRowDataSize(x);
	uint32 subResource = D3D11CalcSubresource(mipLevel, 0, m_MipLevels);

	if (m_Usage == TextureUsage::STATIC)
	{
		if (IsCompressed())
		{
			levelHeight = (levelHeight + 3) >> 2;
		}

		D3D11_BOX box;
		box.back = (uint32)(z + depth);
		box.front = (uint32)z;
		box.left = (uint32)x;
		box.top = (uint32)y;
		box.right = (uint32)(x + width);
		box.bottom = (uint32)(y + height);
		m_pGraphics->GetImpl()->GetDeviceContext()->UpdateSubresource((ID3D11Buffer*)m_pResource, subResource, &box, pData, rowSize, levelHeight * rowSize);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE mapData = {};
		HR(m_pGraphics->GetImpl()->GetDeviceContext()->Map((ID3D11Buffer*)m_pResource, subResource, D3D11_MAP_WRITE_DISCARD, 0, &mapData));
		for (int depthIndex = 0; depthIndex < depth; ++depthIndex)
		{
			for (int rowIdx = 0; rowIdx < height; ++rowIdx)
			{
				memcpy((char*)mapData.pData + (depthIndex + z) * mapData.DepthPitch + (rowIdx + y) * mapData.RowPitch + rowStart, (char*)pData + rowIdx * rowSize, rowSize);
			}
		}
		m_pGraphics->GetImpl()->GetDeviceContext()->Unmap((ID3D11Buffer*)m_pResource, subResource);
	}

	SetMemoryUsage(m_Height * m_Width * m_Depth * 4);

	return true;
}

bool Texture3D::SetImage(const Image& image)
{
	AUTOPROFILE(Texture3D_SetData_Image);

	uint32 memoryUsage = 0;

	ImageFormat compressionFormat = image.GetFormat();
	m_MipLevels = image.GetMipLevels();
	if (!SetSize(image.GetWidth(), image.GetHeight(), image.GetDepth(), TextureFormatFromCompressionFormat(compressionFormat, image.IsSRGB()), TextureUsage::STATIC, 1, nullptr))
	{
		return false;
	}

	AUTOPROFILE_DESC(Texture3D_SetImage_Compressed, "Compressed load");
	for (int mipLevel = 0; mipLevel < image.GetMipLevels(); ++mipLevel)
	{
		MipLevelInfo mipData = image.GetMipInfo(mipLevel);
		if (!SetData(mipLevel, 0, 0, 0, mipData.Width, mipData.Height, mipData.Depth, image.GetData(mipLevel)))
		{
			return false;
		}
		memoryUsage += mipData.DataSize;
	}

	SetMemoryUsage(memoryUsage);
	return true;
}

bool Texture3D::Create()
{
	AUTOPROFILE(Texture3D_Create);

	D3D11_TEXTURE3D_DESC desc;
	memset(&desc, 0, sizeof(D3D11_TEXTURE3D_DESC));
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = m_Usage == TextureUsage::DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.Format = (DXGI_FORMAT)m_TextureFormat;
	desc.Height = m_Height;
	desc.Width = m_Width;
	desc.Depth = m_Depth;
	desc.MipLevels = m_MipLevels;
	desc.MiscFlags = 0;
	desc.Usage = (m_Usage == TextureUsage::DYNAMIC) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	if (m_pResource == nullptr)
	{
		HR(m_pGraphics->GetImpl()->GetDevice()->CreateTexture3D(&desc, nullptr, (ID3D11Texture3D**)&m_pResource));
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MipLevels = m_MipLevels;
	srvDesc.Format = (DXGI_FORMAT)GetSRVFormat(m_TextureFormat);

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateShaderResourceView((ID3D11Texture3D*)m_pResource, &srvDesc, (ID3D11ShaderResourceView**)&m_pShaderResourceView));

	return true;
}
