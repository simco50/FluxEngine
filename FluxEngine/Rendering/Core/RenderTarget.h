#pragma once
class Texture;
class Graphics;

enum class ClearFlags;

struct RENDER_TARGET_DESC
{
	bool DepthBuffer = true;
	bool ColorBuffer = true;

	int Width = -1;
	int Height = -1;
	int MsaaSampleCount = 1;

	DXGI_FORMAT DepthFormat = DXGI_FORMAT_R24G8_TYPELESS;
	DXGI_FORMAT ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	void* pColor = nullptr;
	void* pDepth = nullptr;

	bool IsValid() const
	{
		if (Width < 0 || Height < 0)
		{
			FLUX_LOG(ERROR, "[RENDER_TARGET_DESC::IsValid()] > RenderTarget dimensions invalid!");
			return false;
		}
		if (!ColorBuffer && !DepthBuffer)
		{
			FLUX_LOG(ERROR, "[RENDER_TARGET_DESC::IsValid()] > RenderTarget needs at least one buffer to create!");
			return false;
		}
		if (MsaaSampleCount < 1)
		{
			FLUX_LOG(ERROR, "[RENDER_TARGET_DESC::IsValid()] > MSAA Sample count has to be at least 1 (is %i)", MsaaSampleCount);
			return false;
		}
		return true;
	}
};

class RenderTarget
{
public:
	RenderTarget(Graphics* pGraphics);
	~RenderTarget();

	DELETE_COPY(RenderTarget)

	void Clear(const ClearFlags clearFlags, const Color& color, const float depth, unsigned char stencil);

	bool Create(const RENDER_TARGET_DESC& RenderTargetDesc);

	Texture* GetDepthTexture() const { return m_pDepthTexture.get(); }
	Texture* GetRenderTexture() const { return m_pRenderTexture.get(); }

private:
	Graphics* m_pGraphics;

	unique_ptr<Texture> m_pRenderTexture;
	unique_ptr<Texture> m_pDepthTexture;
};
