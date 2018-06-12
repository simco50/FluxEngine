#include "FluxEngine.h"
#include "../TextureCube.h"
#include "../Graphics.h"

#include "D3D11GraphicsImpl.h"
#include "Content/Image.h"

TextureCube::TextureCube(Context* pContext) :
	Texture(pContext)
{

}

TextureCube::~TextureCube()
{

}

bool TextureCube::Load(InputStream& inputStream)
{
	AUTOPROFILE(TextureCube_Load);

	m_pImage = std::make_unique<Image>(m_pContext);
	if (m_pImage->Load(inputStream) == false)
	{
		return false;
	}
	if (SetImageChain(*m_pImage) == false)
	{
		return false;
	}
	return true;
}

bool TextureCube::SetSize(const int width, const int height, const unsigned int format, TextureUsage usage, const int multiSample, void* pTexture)
{
	if (multiSample > 1 && usage != TextureUsage::DEPTHSTENCILBUFFER && usage != TextureUsage::RENDERTARGET)
	{
		FLUX_LOG(Error, "[Texture::SetSize()] > Multisampling is only supported for rendertarget or depth-stencil textures");
		return false;
	}

	//If all the properties are the same, we can safely say it's not necessary to create a new texture except when there is a change in mip levels
	if (m_Width == (uint32)width && m_Height == (uint32)height && m_TextureFormat == format && m_Usage == usage && m_MultiSample == (uint32)multiSample)
	{
		return true;
	}

	AUTOPROFILE(TextureCube_SetSize);
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

bool TextureCube::SetData(const CubeMapFace face, const unsigned int mipLevel, int x, int y, int width, int height, const void* pData)
{
	AUTOPROFILE(TextureCube_SetData);

	int32 levelWidth = GetLevelWidth(mipLevel);
	int32 levelHeight = GetLevelHeight(mipLevel);

	if (x < 0 || x + width > levelWidth || y < 0 || y + height > levelHeight || width <= 0 || height <= 0)
	{
		FLUX_LOG(Warning, "[Texture3D::SetData] Illegal dimensions for setting data");
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
	uint32 subResource = D3D11CalcSubresource(mipLevel, (uint32)face, m_MipLevels);

	if (m_Usage == TextureUsage::STATIC)
	{
		if (IsCompressed())
		{
			levelHeight = (levelHeight + 3) >> 2;
		}

		D3D11_BOX box;
		box.back = 1;
		box.front = 0;
		box.left = (uint32)x;
		box.top = (uint32)y;
		box.right = (uint32)(x + width);
		box.bottom = (uint32)(y + height);
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

	SetMemoryUsage(m_Height * m_Width * m_Depth * 4);

	return true;
}

bool TextureCube::SetImage(const CubeMapFace face, const Image& image)
{
	AUTOPROFILE(TextureCube_SetData_Image);

	ImageFormat compressionFormat = image.GetFormat();
	m_MipLevels = image.GetMipLevels();
	if (!SetSize(image.GetWidth(), image.GetHeight(), TextureFormatFromCompressionFormat(compressionFormat, image.IsSRGB()), TextureUsage::STATIC, 1, nullptr))
	{
		return false;
	}

	AUTOPROFILE_DESC(TextureCube_SetImage_Compressed, "Compressed load");
	for (int mipLevel = 0; mipLevel < image.GetMipLevels(); ++mipLevel)
	{
		MipLevelInfo mipData = image.GetMipInfo(mipLevel);
		if (!SetData(face, mipLevel, 0, 0, mipData.Width, mipData.Height, image.GetData(mipLevel)))
		{
			return false;
		}
	}

	return true;
}

bool TextureCube::SetImageChain(const Image& image)
{
	AUTOPROFILE(TextureCube_SetImageChain);

	const Image* pCurrentImage = &image;
	for (uint32 faceIdx = 0; faceIdx < (uint32)CubeMapFace::MAX, pCurrentImage != nullptr; ++faceIdx)
	{
		SetImage((CubeMapFace)faceIdx, *pCurrentImage);
		pCurrentImage = pCurrentImage->GetNextImage();
	}

	return true;
}

bool TextureCube::Create()
{
	AUTOPROFILE(TextureCube_Create);

	D3D11_TEXTURE2D_DESC desc = {};
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = m_Usage == TextureUsage::DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.Format = (DXGI_FORMAT)m_TextureFormat;
	desc.Height = m_Height;
	desc.Width = m_Width;
	desc.MipLevels = m_MipLevels;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	desc.Usage = (m_Usage == TextureUsage::DYNAMIC) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.ArraySize = (int)CubeMapFace::MAX;
	if (m_pResource == nullptr)
	{
		HR(m_pGraphics->GetImpl()->GetDevice()->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)&m_pResource));
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = m_MipLevels;
	srvDesc.Format = (DXGI_FORMAT)GetSRVFormat(m_TextureFormat);
	
	HR(m_pGraphics->GetImpl()->GetDevice()->CreateShaderResourceView((ID3D11Texture2D*)m_pResource, &srvDesc, (ID3D11ShaderResourceView**)&m_pShaderResourceView));

	return true;
}
