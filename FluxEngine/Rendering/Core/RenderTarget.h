#pragma once
class Texture2D;
class Graphics;

enum class ClearFlags;

struct RenderTargetDesc
{
	bool DepthBuffer = true;
	bool ColorBuffer = true;

	int Width = -1;
	int Height = -1;
	int MultiSample = 1;

	DXGI_FORMAT DepthFormat = DXGI_FORMAT_R24G8_TYPELESS;
	DXGI_FORMAT ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	void* pColorResource = nullptr;
	void* pDepthResource = nullptr;
};

class RenderTarget : public Object
{
	FLUX_OBJECT(RenderTarget, Object)

public:
	RenderTarget(Context* pContext);
	~RenderTarget();

	DELETE_COPY(RenderTarget)

	bool Create(const RenderTargetDesc& RenderTargetDesc);

	Texture2D* GetDepthTexture() const { return m_pDepthTexture.get(); }
	Texture2D* GetRenderTexture() const { return m_pRenderTexture.get(); }

private:
	bool ValidateDesc(const RenderTargetDesc& desc) const;

	Graphics* m_pGraphics;

	std::unique_ptr<Texture2D> m_pRenderTexture;
	std::unique_ptr<Texture2D> m_pDepthTexture;
};
