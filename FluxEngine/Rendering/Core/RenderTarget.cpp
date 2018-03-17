#include "FluxEngine.h"
#include "RenderTarget.h"
#include "Graphics.h"
#include "Texture2D.h"

RenderTarget::RenderTarget(Context* pContext) :
	Object(pContext)
{
	m_pGraphics = pContext->GetSubsystem<Graphics>();
}

RenderTarget::~RenderTarget()
{
}

bool RenderTarget::Create(const RenderTargetDesc& RenderTargetDesc)
{
	AUTOPROFILE(RenderTarget_Create);

	if (!ValidateDesc(RenderTargetDesc))
		return false;

	m_pRenderTexture.reset();
	m_pDepthTexture.reset();
	m_pRenderTexture = std::make_unique<Texture2D>(m_pContext);
	if (!m_pRenderTexture->SetSize(RenderTargetDesc.Width, RenderTargetDesc.Height, RenderTargetDesc.ColorFormat, TextureUsage::RENDERTARGET, RenderTargetDesc.MultiSample, RenderTargetDesc.pColorResource))
		return false;

	m_pDepthTexture = std::make_unique<Texture2D>(m_pContext);
	if (!m_pDepthTexture->SetSize(RenderTargetDesc.Width, RenderTargetDesc.Height, RenderTargetDesc.DepthFormat, TextureUsage::DEPTHSTENCILBUFFER, RenderTargetDesc.MultiSample, RenderTargetDesc.pDepthResource))
		return false;

	return true;
}

bool RenderTarget::ValidateDesc(const RenderTargetDesc& desc) const
{
	if (desc.Width < 0 || desc.Height < 0)
	{
		FLUX_LOG(Error, "[RenderTarget::ValidateDesc] > RenderTarget dimensions (%i x %i) invalid", desc.Width, desc.Height);
		return false;
	}
	if (!desc.ColorBuffer && !desc.DepthBuffer)
	{
		FLUX_LOG(Error, "[RenderTarget::ValidateDesc] > RenderTarget needs at least one buffer to create");
		return false;
	}
	if (desc.MultiSample < 1)
	{
		FLUX_LOG(Error, "[RenderTarget::ValidateDesc] > MSAA Sample count has to be at least 1 (is %i)", desc.MultiSample);
		return false;
	}
	return true;
}
