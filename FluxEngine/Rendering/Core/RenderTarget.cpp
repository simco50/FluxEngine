#include "FluxEngine.h"
#include "RenderTarget.h"

RenderTarget::RenderTarget(Texture* pTexture) :
	m_pParentTexture(pTexture)
{
}

RenderTarget::~RenderTarget()
{
	Release();
}

void RenderTarget::Release()
{
	if (m_pRenderTargetView)
	{
		SafeRelease(m_pRenderTargetView);
	}
}
