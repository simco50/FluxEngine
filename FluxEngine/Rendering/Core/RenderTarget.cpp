#include "stdafx.h"
#include "RenderTarget.h"
#include "Texture.h"
#include "Graphics.h"


RenderTarget::RenderTarget(Graphics* pGraphics) :
	m_pGraphics(pGraphics)
{
}

RenderTarget::~RenderTarget()
{
}

bool RenderTarget::Create(const RENDER_TARGET_DESC& RenderTargetDesc)
{
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

void RenderTarget::ClearColor(const XMFLOAT4& color)
{
	if(m_pRenderTexture)
		m_pGraphics->GetDeviceContext()->ClearRenderTargetView((ID3D11RenderTargetView*)m_pRenderTexture->GetRenderTargetView(), reinterpret_cast<const float*>(&color));
}

void RenderTarget::ClearDepth(const unsigned int depthflags /*= D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL*/, const float depth /*= 1.0f*/, unsigned char stencil /*= 0*/)
{
	if(m_pDepthTexture)
		m_pGraphics->GetDeviceContext()->ClearDepthStencilView((ID3D11DepthStencilView*)m_pDepthTexture->GetRenderTargetView(), (D3D11_CLEAR_FLAG)depthflags, depth, stencil);
}