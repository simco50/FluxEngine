#include "FluxEngine.h"
#include "Texture.h"
#include "Graphics.h"

#include "External/Stb/stb_image.h"
#include "Content/Image.h"

Texture::Texture(Context* pContext) :
	Resource(pContext)
{
	m_pGraphics = pContext->GetSubsystem<Graphics>();
}

Texture::~Texture()
{
	Release();
}

void Texture::SetAddressMode(const TextureAddressMode addressMode)
{
	if (m_AddressMode != addressMode)
	{
		m_AddressMode = addressMode;
		m_ParametersDirty = true;
	}
}

void Texture::Release()
{
	SafeRelease(m_pResource);
	SafeRelease(m_pShaderResourceView);
	SafeRelease(m_pRenderTargetView);
	SafeRelease(m_pReadOnlyView);
	SafeRelease(m_pSamplerState);
	SafeRelease(m_pResolvedResource);
}

int Texture::GetLevelWidth(unsigned int mipLevel)
{
	if (mipLevel > m_MipLevels)
		return 0;
	return Math::Max((int)(m_Width >> mipLevel), 1);
}

int Texture::GetLevelHeight(unsigned int mipLevel)
{
	if (mipLevel > m_MipLevels)
		return 0;
	return Math::Max((int)(m_Height >> mipLevel), 1);
}

int Texture::GetLevelDepth(unsigned int mipLevel)
{
	if (mipLevel > m_MipLevels)
		return 0;
	return Math::Max((int)(m_Depth >> mipLevel), 1);
}