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

ID3D11RenderTargetView* RenderTarget::GetRenderTargetView() const
{
	return (ID3D11RenderTargetView*)m_pRenderTexture->GetRenderTargetView();
}

ID3D11DepthStencilView* RenderTarget::GetDepthStencilView() const
{
	return (ID3D11DepthStencilView*)m_pDepthTexture->GetRenderTargetView();
}

void RenderTarget::ClearColor(const XMFLOAT4& color)
{
	if(m_pRenderTexture)
		m_pGraphics->GetDeviceContext()->ClearRenderTargetView(GetRenderTargetView(), reinterpret_cast<const float*>(&color));
}

void RenderTarget::ClearDepth(const unsigned int depthflags /*= D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL*/, const float depth /*= 1.0f*/, unsigned char stencil /*= 0*/)
{
	if(m_pDepthTexture)
		m_pGraphics->GetDeviceContext()->ClearDepthStencilView(GetDepthStencilView(), (D3D11_CLEAR_FLAG)depthflags, depth, stencil);
}

ID3D11ShaderResourceView* RenderTarget::GetColorSRV() const
{
	return (ID3D11ShaderResourceView*)m_pRenderTexture->GetResourceView();
}

ID3D11ShaderResourceView* RenderTarget::GetDepthSRV() const
{
	return (ID3D11ShaderResourceView*)m_pDepthTexture->GetResourceView();
}

ID3D11Texture2D* RenderTarget::GetColorBuffer() const
{
	return (ID3D11Texture2D*)m_pRenderTexture->GetResource();
}

ID3D11Texture2D* RenderTarget::GetDepthStencilBuffer() const
{
	return (ID3D11Texture2D*)m_pDepthTexture->GetResource();
}
