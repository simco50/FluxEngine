#include "D3D11GraphicsImpl.h"

void RenderTarget::Clear(const ClearFlags clearFlags, const XMFLOAT4& color, const float depth, unsigned char stencil)
{
	if (m_pRenderTexture && (unsigned int)clearFlags & (unsigned int)ClearFlags::RenderTarget)
		m_pGraphics->GetImpl()->GetDeviceContext()->ClearRenderTargetView(
			(ID3D11RenderTargetView*)m_pRenderTexture->GetRenderTargetView(), 
			reinterpret_cast<const float*>(&color));

	unsigned int flags = 0;
	if((unsigned int)clearFlags & (unsigned int)ClearFlags::Stencil)
		flags |= D3D11_CLEAR_STENCIL;
	if ((unsigned int)clearFlags & (unsigned int)ClearFlags::Depth)
		flags |= D3D11_CLEAR_DEPTH;

	if (m_pDepthTexture && flags != 0)
		m_pGraphics->GetImpl()->GetDeviceContext()->ClearDepthStencilView(
			(ID3D11DepthStencilView*)m_pDepthTexture->GetRenderTargetView(), 
			(D3D11_CLEAR_FLAG)flags,
			depth, 
			stencil);
}