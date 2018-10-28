#include "FluxEngine.h"
#include "../Texture.h"
#include "../Graphics.h"
#include "D3D11GraphicsImpl.h"
#include "Content/Image.h"

void Texture::UpdateParameters()
{
	check(m_pResource);
	if (m_pSamplerState && !m_ParametersDirty)
	{
		return;
	}

	AUTOPROFILE(Texture_CreateTextureSampler);

	SafeRelease(m_pSamplerState);

	D3D11_SAMPLER_DESC desc = {};
	Color borderColor = Color();
	switch (m_AddressMode)
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
	case TextureAddressMode::MAX:
	default:
		FLUX_LOG(Warning, "[Texture::UpdateParameters()] Address Mode unknown");
	}

	memcpy(&desc.BorderColor[0], &borderColor.x, 4 * sizeof(float));
	desc.ComparisonFunc = D3D11ComparisonFunction(CompareMode::LESSEQUAL);
	desc.Filter = D3D11Filter(TextureFilter::MIN_MAG_MIP_LINEAR);
	desc.MaxAnisotropy = 1;
	desc.MinLOD = std::numeric_limits<float>::min();
	desc.MaxLOD = std::numeric_limits<float>::max();

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateSamplerState(&desc, (ID3D11SamplerState**)&m_pSamplerState));

	m_ParametersDirty = false;
}

int Texture::GetRowDataSize(unsigned int width) const
{
	switch (m_TextureFormat)
	{
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_A8_UNORM:
		return (unsigned)width;

	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_R16_TYPELESS:
		return (unsigned)(width * 2);

	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_R32_TYPELESS:
		return (unsigned)(width * 4);

	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return (unsigned)(width * 8);

	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return (unsigned)(width * 16);

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return (unsigned)(((width + 3) >> 2) * 8);

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return (unsigned)(((width + 3) >> 2) * 16);

	default:
		checkf(false, "Texture format not supported");
		return 0;
	}
}

unsigned int Texture::GetSRVFormat(const unsigned int format)
{
	if (format == DXGI_FORMAT_R24G8_TYPELESS)
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	if (format == DXGI_FORMAT_R16_TYPELESS)
		return DXGI_FORMAT_R16_UNORM;
	if (format == DXGI_FORMAT_R32_TYPELESS)
		return DXGI_FORMAT_R32_FLOAT;
	return format;
}

unsigned int Texture::GetDSVFormat(const unsigned int format)
{
	if (format == DXGI_FORMAT_R24G8_TYPELESS)
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	if (format == DXGI_FORMAT_R16_TYPELESS)
		return DXGI_FORMAT_D16_UNORM;
	if (format == DXGI_FORMAT_R32_TYPELESS)
		return DXGI_FORMAT_D32_FLOAT;
	return format;
}

unsigned int Texture::TextureFormatFromCompressionFormat(const ImageFormat& format, bool sRgb)
{
	switch (format)
	{
	case ImageFormat::RGBA:
		if (sRgb)
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case ImageFormat::BGRA:
		if (sRgb)
			return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case ImageFormat::DXT1:
		if (sRgb)
			return DXGI_FORMAT_BC1_UNORM_SRGB;
		return DXGI_FORMAT_BC1_UNORM;
	case ImageFormat::DXT3:
		if (sRgb)
			return DXGI_FORMAT_BC2_UNORM_SRGB;
		return DXGI_FORMAT_BC2_UNORM;
	case ImageFormat::DXT5:
		if (sRgb)
			return DXGI_FORMAT_BC3_UNORM_SRGB;
		return DXGI_FORMAT_BC3_UNORM;
	case ImageFormat::BC4:
		return DXGI_FORMAT_BC4_UNORM;
	case ImageFormat::BC5:
		return DXGI_FORMAT_BC5_UNORM;
	case ImageFormat::BC6H:
		return DXGI_FORMAT_BC6H_UF16;
	case ImageFormat::BC7:
		if (sRgb)
			return DXGI_FORMAT_BC7_UNORM_SRGB;
		return DXGI_FORMAT_BC7_UNORM;
	default:
		checkf(false, "Texture format not supported");
		return 0;
	}
}

bool Texture::IsCompressed() const
{
	switch (m_TextureFormat)
	{
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return true;
	default:
		return false;
	}
}

void Texture::RegenerateMips()
{
	if (m_Usage == TextureUsage::RENDERTARGET && m_pShaderResourceView != nullptr && m_MipLevels > 1)
	{
		m_pGraphics->GetImpl()->GetDeviceContext()->GenerateMips((ID3D11ShaderResourceView*)m_pShaderResourceView);
	}
}