#pragma once
#include "Helpers/Singleton.h"

struct RenderItem;
class RenderTarget;

struct RendererDesc
{
	HWND Hwnd;
	int WindowHeight = 1280;
	int WindowWidth = 720;
	bool Msaa = false;
	bool VerticalSync = true;
	XMFLOAT4 ClearColor = (XMFLOAT4)DirectX::Colors::DarkGray;
	WindowStyle WindowStyle = WindowStyle::WINDOWED;

	UINT MsaaQuality;
};

class Renderer : public Singleton<Renderer>
{
public:

	HRESULT Initialize(const RendererDesc& desc);
	HRESULT Shutdown();

	void NewFrame();
	void Submit(const RenderItem& item);
	void Render();
	void RenderImmediate(const RenderItem& item);
	void OnResize();

	void Present();

	RendererDesc& GetDesc() { return m_RendererDesc; }

	ID3D11Device* GetDevice() const { return m_pDevice.Get(); }
	ID3D11DeviceContext* GetDeviceContext() const { return m_pDeviceContext.Get(); }
	RenderTarget* GetDefaultRenderTarget() const { return m_pDefaultRenderTarget; }

public:
	void SetInputLayout(ID3D11InputLayout* pInputLayout);
	void SetPrimitiveTopology(const D3D11_PRIMITIVE_TOPOLOGY& topology);
	void SetVertexBuffer(const int startSlot, ID3D11Buffer* pVertexBuffer, UINT* pStrides, UINT* pOffset);
	void SetVertexBuffers(const int startSlot, const vector<ID3D11Buffer*>& pVertexBuffers, UINT* pStrides, UINT* pOffset);
	void SetIndexBuffer(ID3D11Buffer* pIndexBuffer, const DXGI_FORMAT format = DXGI_FORMAT_R32_UINT, const UINT offset = 0);
private:
	RendererDesc m_RendererDesc;

	struct RendererState
	{
		ID3D11Buffer* VertexBuffer = nullptr;
		ID3D11Buffer* IndexBuffer = nullptr;
		ID3D11InputLayout* InputLayout = nullptr;
		D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	};

	RendererState m_CurrentRenderState = {};

	HRESULT CreateSwapchain();
	HRESULT EnumerateAdapters();
	HRESULT CreateDevice();

	void SetRenderTargets(const vector<RenderTarget*>& renderTargets);
	void SetRenderTarget(RenderTarget* pRenderTarget);

	void RenderSingle(const RenderItem& item);
	void FlushQueue();
	void SortQueue();

	vector<RenderItem> m_RenderQueue;

	RenderTarget* m_pDefaultRenderTarget;

	D3D11_VIEWPORT m_Viewport;

	//Direct3D
	Smart_COM::Unique_COM<IDXGIAdapter> m_pAdapter;
	Smart_COM::Unique_COM<ID3D11Device> m_pDevice;
	Smart_COM::Unique_COM<ID3D11DeviceContext> m_pDeviceContext;
	Smart_COM::Unique_COM<IDXGIFactory> m_pFactory;
	Smart_COM::Unique_COM<IDXGISwapChain> m_pSwapChain;

	//Constants
	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
};