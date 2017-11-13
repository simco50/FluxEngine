#include "stdafx.h"
#include "RenderTarget.h"
#include "Texture.h"
#include "Graphics.h"

#ifdef D3D11
#include "D3D11/D3D11RenderTarget.hpp"
#endif

RenderTarget::RenderTarget(Graphics* pGraphics) :
	m_pGraphics(pGraphics)
{
}

RenderTarget::~RenderTarget()
{
}

bool RenderTarget::Create(const RENDER_TARGET_DESC& RenderTargetDesc)
{
	AUTOPROFILE(RenderTarget_Create);

	if (!RenderTargetDesc.IsValid())
		return false;

	m_pRenderTexture.reset();
	m_pDepthTexture.reset();
	m_pRenderTexture = unique_ptr<Texture>(new Texture(m_pGraphics));
	if (!m_pRenderTexture->SetSize(RenderTargetDesc.Width, RenderTargetDesc.Height, RenderTargetDesc.ColorFormat, TextureUsage::RENDERTARGET, RenderTargetDesc.MsaaSampleCount, RenderTargetDesc.pColor))
		return false;

	m_pDepthTexture = unique_ptr<Texture>(new Texture(m_pGraphics));
	if (!m_pDepthTexture->SetSize(RenderTargetDesc.Width, RenderTargetDesc.Height, RenderTargetDesc.DepthFormat, TextureUsage::DEPTHSTENCILBUFFER, RenderTargetDesc.MsaaSampleCount, RenderTargetDesc.pDepth))
		return false;

	return true;
}