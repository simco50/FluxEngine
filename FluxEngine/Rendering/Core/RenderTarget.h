#pragma once
struct RENDER_TARGET_DESC
{
	bool DepthBuffer = true;
	bool ColorBuffer = true;
	bool DepthSRV = false;
	bool ColorSRV = false;

	int Width = -1;
	int Height = -1;
	int MsaaSampleCount = 1;
	int MsaaQuality = 0;

	DXGI_FORMAT DepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	ID3D11Texture2D* pColor = nullptr;
	ID3D11Texture2D* pDepth = nullptr;

	bool IsValid()
	{
		if (DepthSRV && DepthBuffer == false)
		{
			FLUX_LOG(ERROR, "[RENDER_TARGET_DESC::IsValid()] > Can't create DepthSRV without a DepthBuffer");
			return false;
		}
		if(ColorSRV && ColorBuffer == false)
		{
			FLUX_LOG(ERROR, "[RENDER_TARGET_DESC::IsValid()] > Can't create ColorSRV without a ColorBuffer");
			return false;
		}
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
		return true;
	}
};

class RenderTarget
{
public:
	RenderTarget(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	~RenderTarget();

	void ClearColor(const XMFLOAT4& color = (XMFLOAT4)DirectX::Colors::CornflowerBlue);
	void ClearDepth(const unsigned int depthflags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, const float depth = 1.0f, unsigned char stencil = 0);

	void Create(const RENDER_TARGET_DESC& RenderTargetDesc);

	ID3D11RenderTargetView* GetRenderTargetView() const { return m_pRenderTargetView.Get(); }
	ID3D11DepthStencilView* GetDepthStencilView() const { return m_pDepthStencilView.Get(); }

	ID3D11ShaderResourceView* GetColorSRV() const;
	ID3D11ShaderResourceView* GetDepthSRV() const;

	ID3D11Texture2D* GetColorBuffer() const { return m_pColorBuffer.Get(); }
	ID3D11Texture2D* GetDepthStencilBuffer() const { return m_pDepthBuffer.Get(); }

private:
	void Reset();
	void CreateColor();
	void CreateDepth();

	DXGI_FORMAT GetDepthResourceFormat(DXGI_FORMAT initFormat);
	DXGI_FORMAT GetDepthSRVFormat(DXGI_FORMAT initFormat);

	ID3D11Device* m_pD3Device;
	ID3D11DeviceContext* m_pD3DeviceContext;

	RENDER_TARGET_DESC m_RenderTargetDesc;

	Unique_COM<ID3D11RenderTargetView> m_pRenderTargetView;
	Unique_COM<ID3D11ShaderResourceView> m_pColorSRV;
	Unique_COM<ID3D11Texture2D> m_pColorBuffer;

	Unique_COM<ID3D11DepthStencilView> m_pDepthStencilView;
	Unique_COM<ID3D11ShaderResourceView> m_pDepthSRV;
	Unique_COM<ID3D11Texture2D> m_pDepthBuffer;
};
