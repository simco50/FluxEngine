#pragma once
struct RENDER_TARGET_DESC
{
	bool DepthBuffer = true;
	bool ColorBuffer = true;
	bool DepthSRV = false;
	bool ColorSRV = false;

	int Width = -1;
	int Height = -1;
	bool MSAA = false;
	int MsaaQuality = 0;

	DXGI_FORMAT DepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	ID3D11Texture2D* pColor = nullptr;
	ID3D11Texture2D* pDepth = nullptr;

	XMFLOAT4 ClearColor = (XMFLOAT4)Colors::CornflowerBlue;

	bool IsValid()
	{
		if (DepthSRV && DepthBuffer == false)
		{
			Console::Log("RENDER_TARGET_DESC::IsValid() -> Can't create DepthSRV without a DepthBuffer", LogType::ERROR);
			return false;
		}
		if(ColorSRV && ColorBuffer == false)
		{
			Console::Log("RENDER_TARGET_DESC::IsValid() -> Can't create ColorSRV without a ColorBuffer", LogType::ERROR);
			return false;
		}
		if(Width < 0 || Height < 0)
		{
			Console::Log("RENDER_TARGET_DESC::IsValid() -> RenderTarget dimensions invalid!", LogType::ERROR);
			return false;
		}
		if(!ColorBuffer && !DepthBuffer)
		{
			Console::Log("RENDER_TARGET_DESC::IsValid() -> RenderTarget needs at least one buffer to create!", LogType::ERROR);
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

	void ClearColor();
	void ClearDepth();

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
