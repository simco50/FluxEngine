#include "D3D11GraphicsImpl.h"
void RenderTarget::ClearColor(const XMFLOAT4& color)
{
	if (m_pRenderTexture)
		m_pGraphics->GetImpl()->GetDeviceContext()->ClearRenderTargetView((ID3D11RenderTargetView*)m_pRenderTexture->GetRenderTargetView(), reinterpret_cast<const float*>(&color));
}

void RenderTarget::ClearDepth(const unsigned int depthflags /*= D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL*/, const float depth /*= 1.0f*/, unsigned char stencil /*= 0*/)
{
	if (m_pDepthTexture)
		m_pGraphics->GetImpl()->GetDeviceContext()->ClearDepthStencilView((ID3D11DepthStencilView*)m_pDepthTexture->GetRenderTargetView(), (D3D11_CLEAR_FLAG)depthflags, depth, stencil);
}