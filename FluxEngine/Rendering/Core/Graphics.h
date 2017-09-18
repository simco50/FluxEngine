#pragma once

class VertexBuffer;
class IndexBuffer;
class RenderTarget;
class ShaderVariation;

enum class PrimitiveType
{
	TRIANGLELIST,
	POINTLIST,
	TRIANGLESTRIP,
	UNDEFINED,
};

class Graphics
{
public:
	Graphics();
	~Graphics();

	bool SetMode(
		const int width, 
		const int height, 
		const bool fullscreen, 
		const bool borderless, 
		const bool resizable, 
		const bool vsync,
		const int multiSample,
		const int monitor,
		const int refreshRate);

	void SetWindowTitle(const string& title);

	void SetWindowPosition(const XMFLOAT2& position);

	void SetRenderTarget(RenderTarget* pRenderTarget);
	void SetRenderTargets(const vector<RenderTarget*>& pRenderTargets);

	void SetVertexBuffer(VertexBuffer* pBuffer);
	void SetVertexBuffers(const vector<VertexBuffer*>& pBuffers);

	void SetIndexBuffer(IndexBuffer* pIndexBuffer);

	void SetShaders(ShaderVariation* pVertexShader, ShaderVariation* pPixelShader);

	void Draw(const PrimitiveType type, const int vertexStart, const int vertexCount);
	void Draw(const PrimitiveType type, const int indexStart, const int indexCount, const int minVertex, const int vertexCount);

	void Clear(const unsigned int flags, const XMFLOAT4& color = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), const float depth = 1.0f, const unsigned int stencil = 0);

	void BeginFrame();

	void EndFrame();

	void OnPause(const bool paused);

	//Getters
	const HWND& GetHwnd() const { return m_Hwnd; }
	int GetWindowWidth() const { return m_WindowWidth; }
	int GetWindowHeight() const { return m_WindowHeight; }

private:
	void SetPrimitiveType(const PrimitiveType type);

	bool RegisterWindowClass();
	bool MakeWindow();
	bool EnumerateAdapters();
	bool CreateDevice(const int windowWidth, const int windowHeight, const int multiSample);
	bool UpdateSwapchain(const int windowWidth, const int windowHeight);

	static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	int m_WindowWidth = 1240;
	int m_WindowHeight = 720;
	HWND m_Hwnd;
	HINSTANCE m_hInstance;
	bool m_Fullscreen = false;
	bool m_Borderless = false;
	bool m_Resizable = true;
	bool m_Vsync = true;
	int m_Multisample = 0;
	int m_MsaaQuality = -1;
	int m_Monitor = 0;
	int m_RefreshRate = 60;
	string m_WindowTitle;

	bool m_Paused = false;
	bool m_Maximized = false;
	bool m_Minimized = false;
	bool m_Resizing = false;

	RenderTarget* m_DefaultRenderTarget = nullptr;

	//Direct3D
	Smart_COM::Unique_COM<IDXGIAdapter> m_pAdapter;
	Smart_COM::Unique_COM<ID3D11Device> m_pDevice;
	Smart_COM::Unique_COM<ID3D11DeviceContext> m_pDeviceContext;
	Smart_COM::Unique_COM<IDXGIFactory> m_pFactory;
	Smart_COM::Unique_COM<IDXGISwapChain> m_pSwapChain;

	D3D11_VIEWPORT m_Viewport;

	//Current State
	IndexBuffer* m_pCurrentIndexBuffer = nullptr;
	VertexBuffer* m_pCurrentVertexBuffer = nullptr;
	RenderTarget* m_pCurrentRenderTarget = nullptr;
	PrimitiveType m_CurrentPrimitiveType = PrimitiveType::UNDEFINED;

	ShaderVariation* m_pCurrentVertexShader = nullptr;
	ShaderVariation* m_pCurrentPixelShader = nullptr;
};

