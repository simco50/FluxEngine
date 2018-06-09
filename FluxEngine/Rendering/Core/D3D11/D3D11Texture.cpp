#include "FluxEngine.h"
#include "../Texture.h"
#include "../Graphics.h"

#include "D3D11GraphicsImpl.h"

#include "External/Stb/stb_image_write.h"

void Texture::UpdateParameters()
{
	if ((m_pSamplerState && !m_ParametersDirty) || m_pResource == nullptr)
		return;

	AUTOPROFILE(Texture_CreateTextureSampler);

	SafeRelease(m_pSamplerState);

	D3D11_SAMPLER_DESC desc = {};
	Color borderColor = Color();
	switch (m_TextureAddressMode)
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
	desc.Filter = D3D11Filter(TextureFilter::MIN_MAG_MIP_LINEAR);
	desc.MaxAnisotropy = 1;
	desc.MinLOD = std::numeric_limits<float>::min();
	desc.MaxLOD = std::numeric_limits<float>::max();

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateSamplerState(&desc, (ID3D11SamplerState**)&m_pSamplerState));
}

int Texture::GetRowDataSize(unsigned int width)
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

	case DXGI_FORMAT_BC1_UNORM:
		return (unsigned)(((width + 3) >> 2) * 8);

	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC3_UNORM:
		return (unsigned)(((width + 3) >> 2) * 16);

	default:
		return 0;
	}
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