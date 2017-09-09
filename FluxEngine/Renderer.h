#pragma once
#include "Helpers/Singleton.h"

struct RenderItem;
class RenderTarget;

class Renderer : public Singleton<Renderer>
{
public:
	~Renderer();

	HRESULT Initialize(const HWND& hwnd, const GameSettings& gameSettings);
	HRESULT Shutdown();

	void Submit(const RenderItem* item);
	void Render();
	void OnResize();

private:

	HRESULT CreateSwapchain();
	HRESULT EnumerateAdapters();
	HRESULT CreateDevice();

	void RenderSingle(const RenderItem* item);
	void FlushQueue();
	void SortQueue();

	vector<const RenderItem*> m_RenderQueue;

	ID3D11Device* m_pD3Device = nullptr;
	ID3D11DeviceContext* m_pD3DeviceContext = nullptr;
	HWND m_hwnd = nullptr;
	const GameSettings* m_pGameSettings;
	unique_ptr<RenderTarget> m_pDefaultRenderTarget;

	D3D11_VIEWPORT m_Viewport;

	//Direct3D
	Unique_COM<IDXGIAdapter> m_pAdapter;
	Unique_COM<ID3D11Device> m_pDevice;
	Unique_COM<ID3D11DeviceContext> m_pDeviceContext;
	Unique_COM<IDXGIFactory> m_pFactory;
	Unique_COM<IDXGISwapChain> m_pSwapChain;

	//Constants
	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
};

