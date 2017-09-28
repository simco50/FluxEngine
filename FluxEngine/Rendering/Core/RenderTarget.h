#pragma once
class Texture;
class Graphics;

struct RENDER_TARGET_DESC
{
	bool DepthBuffer = true;
	bool ColorBuffer = true;

	int Width = -1;
	int Height = -1;
	int MsaaSampleCount = 1;

	DXGI_FORMAT DepthFormat = DXGI_FORMAT_R24G8_TYPELESS;
	DXGI_FORMAT ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	ID3D11Texture2D* pColor = nullptr;
	ID3D11Texture2D* pDepth = nullptr;

	bool IsValid() const
	{
		if(Width < 0 || Height < 0)
		{
			FLUX_LOG(ERROR, "[RENDER_TARGET_DESC::IsValid()] > RenderTarget dimensions invalid!");
			return false;
		}
		if(!ColorBuffer && !DepthBuffer)
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

	void ClearColor(const XMFLOAT4& color = (XMFLOAT4)DirectX::Colors::CornflowerBlue);
	void ClearDepth(const unsigned int depthflags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, const float depth = 1.0f, unsigned char stencil = 0);

	bool Create(const RENDER_TARGET_DESC& RenderTargetDesc);

	ID3D11RenderTargetView* GetRenderTargetView() const;
	ID3D11DepthStencilView* GetDepthStencilView() const;

	ID3D11ShaderResourceView* GetColorSRV() const;
	ID3D11ShaderResourceView* GetDepthSRV() const;

	ID3D11Texture2D* GetColorBuffer() const;
	ID3D11Texture2D* GetDepthStencilBuffer() const;

private:
	Graphics* m_pGraphics;

	unique_ptr<Texture> m_pRenderTexture;
	unique_ptr<Texture> m_pDepthTexture;
};
