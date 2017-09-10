#include "stdafx.h"
#include "Texture.h"

Texture::Texture(ID3D11Resource* pTexture, ID3D11ShaderResourceView* pTextureSRV) :
	m_pTexture(pTexture),
	m_pTextureSRV(pTextureSRV)
{
	ID3D11Texture2D* pTex = static_cast<ID3D11Texture2D*>(pTexture);
	D3D11_TEXTURE2D_DESC desc;
	pTex->GetDesc(&desc);
	m_Width = desc.Width;
	m_Height = desc.Height;
}

Texture::~Texture()
{
	SafeRelease(m_pTexture);
	SafeRelease(m_pTextureSRV);
}