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