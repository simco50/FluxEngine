#pragma once

class RenderTarget;
struct GameSettings;
class ImgUIDrawer;
class PhysicsCore;

class FluxCore
{
public:
	FluxCore();

	virtual ~FluxCore();

	int Run(HINSTANCE hInstance);
	static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	EngineContext& GetEngineContext() { return m_EngineContext; }
	RenderTarget* GetDefaultRenderTarget() const { return m_pDefaultRenderTarget.get(); }

	void SetMSAA(bool value);

protected:
	EngineContext m_EngineContext;
	unique_ptr<RenderTarget> m_pDefaultRenderTarget;
	bool m_Paused = false;

	virtual void OnResize();
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HRESULT RegisterWindowClass();
	HRESULT MakeWindow();
	HRESULT EnumAdapters();
	void LogOutputs(IDXGIAdapter* pAdapter, bool logDisplayModes = false);
	void LogDisplayModes(IDXGIOutput* pOutput);
	HRESULT InitializeD3D();
	void InitializeHighDefinitionMouse();
	void CreateSwapChain();
	void CleanupD3D();
	void OnPause(const bool paused);
	void CalculateFrameStats() const;

	void GameLoop();
	virtual void PrepareGame() = 0;
	virtual void Initialize(EngineContext* pEngineContext) = 0;
	virtual void Render() = 0;
	virtual void Update() = 0;

	//Window variables
	HINSTANCE m_hInstance = nullptr;

	std::string m_WindowClassName = "WindowClass1";

	//Direct3D
	Unique_COM<IDXGIAdapter> m_pAdapter;
	Unique_COM<ID3D11Device> m_pDevice;
	Unique_COM<ID3D11DeviceContext> m_pDeviceContext;
	Unique_COM<IDXGIFactory> m_pFactory;
	Unique_COM<IDXGISwapChain> m_pSwapChain;

	D3D11_VIEWPORT m_Viewport;

	//Constants
	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//Window states
	bool m_Minimized = false;
	bool m_Maximized = false;
	bool m_Resizing = false;

	ImgUIDrawer* m_pUIDrawer = nullptr;

	PhysicsCore* m_pPhysicsCore = nullptr;
};
