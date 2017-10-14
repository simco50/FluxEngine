#pragma once
#include "GraphicsDefines.h"

class VertexBuffer;
class IndexBuffer;
class RenderTarget;
class ShaderVariation;
class InputLayout;
class Texture;
class BlendState;
class RasterizerState;
class DepthStencilState;
class ConstantBuffer;
enum class ShaderType : unsigned char;

class Graphics
{
public:
	Graphics(HINSTANCE hInstance);
	~Graphics();

	DELETE_COPY(Graphics)

	bool SetMode(
		const int width, 
		const int height, 
		const WindowType windowType,
		const bool resizable, 
		const bool vsync,
		const int multiSample,
		const int refreshRate);

	void SetWindowTitle(const string& title);

	void SetWindowPosition(const XMFLOAT2& position);

	void SetRenderTarget(RenderTarget* pRenderTarget);
	void SetRenderTargets(const vector<RenderTarget*>& pRenderTargets);

	void SetVertexBuffer(VertexBuffer* pBuffer);
	void SetVertexBuffers(const vector<VertexBuffer*>& pBuffers);

	void SetIndexBuffer(IndexBuffer* pIndexBuffer);

	void SetShaders(ShaderVariation* pVertexShader, ShaderVariation* pPixelShader);

	void SetInputLayout(InputLayout* pInputLayout);

	void SetViewport(const FloatRect& rect);
	void SetScissorRect(const bool enabled, const IntRect& rect = IntRect::ZERO());

	void SetTexture(const unsigned int index, Texture* pTexture);

	void Draw(const PrimitiveType type, const int vertexStart, const int vertexCount);
	void Draw(const PrimitiveType type, const int indexCount, const int indexStart, const int minVertex);

	void Clear(const unsigned int flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, const XMFLOAT4& color = (XMFLOAT4)DirectX::Colors::CornflowerBlue, const float depth = 1.0f, const unsigned char stencil = 0);

	void PrepareDraw();
	void BeginFrame();
	void EndFrame();

	void OnPause(const bool paused);

	//Getters
	const HWND& GetWindow() const { return m_Hwnd; }
	int GetWindowWidth() const { return m_WindowWidth; }
	int GetWindowHeight() const { return m_WindowHeight; }

	BlendState* GetBlendState() const { return m_pBlendState.get(); }
	RasterizerState* GetRasterizerState() const { return m_pRasterizerState.get(); }
	DepthStencilState* GetDepthStencilState() const { return m_pDepthStencilState.get(); }

	ConstantBuffer* GetOrCreateConstantBuffer(unsigned int size, const ShaderType shaderType, unsigned int registerIndex);

	ID3D11Device* GetDevice() const { return m_pDevice.Get(); }
	ID3D11DeviceContext* GetDeviceContext() const { return m_pDeviceContext.Get(); }
	unsigned int GetMultisampleQuality(const DXGI_FORMAT format, const unsigned int sampleCount) const;

private:
	void SetPrimitiveType(const PrimitiveType type);

	bool RegisterWindowClass();
	bool MakeWindow(int windowWidth, int windowHeight);
	bool EnumerateAdapters();
	bool CreateDevice(const int windowWidth, const int windowHeight);
	bool UpdateSwapchain();

	bool CheckMultisampleQuality(const DXGI_FORMAT format, const unsigned int sampleCount) const;

	static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	int m_WindowWidth = 1240;
	int m_WindowHeight = 720;
	HWND m_Hwnd;
	HINSTANCE m_hInstance;
	WindowType m_WindowType = WindowType::WINDOWED;
	bool m_Resizable = true;
	bool m_Vsync = true;
	int m_Multisample = 1;
	int m_RefreshRate = 60;
	string m_WindowTitle;

	bool m_Paused = false;
	bool m_Maximized = false;
	bool m_Minimized = false;
	bool m_Resizing = false;

	ComPtr<IDXGIAdapter> m_pAdapter;
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	ComPtr<IDXGIFactory> m_pFactory;
	ComPtr<IDXGISwapChain> m_pSwapChain;

	unique_ptr<RenderTarget> m_pDefaultRenderTarget;
	unique_ptr<BlendState> m_pBlendState;
	unique_ptr<RasterizerState> m_pRasterizerState;
	unique_ptr<DepthStencilState> m_pDepthStencilState;

	map<unsigned int, unique_ptr<ConstantBuffer>> m_ConstantBuffers;

	//Clip rect
	IntRect m_CurrentScissorRect;
	bool m_ScissorEnabled = false;
	bool m_ScissorRectDirty = true;

	IndexBuffer* m_pCurrentIndexBuffer = nullptr;
	VertexBuffer* m_pCurrentVertexBuffer = nullptr;
	PrimitiveType m_CurrentPrimitiveType = PrimitiveType::UNDEFINED;
	InputLayout* m_pCurrentInputLayout = nullptr;
	FloatRect m_CurrentViewport;

	ShaderVariation* m_pCurrentVertexShader = nullptr;
	ShaderVariation* m_pCurrentPixelShader = nullptr;

	vector<ID3D11SamplerState*> m_CurrentSamplerStates;
	vector<ID3D11ShaderResourceView*> m_CurrentShaderResourceViews;
};