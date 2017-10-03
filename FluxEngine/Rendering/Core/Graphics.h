#pragma once
#include "GraphicsDefines.h"

class VertexBuffer;
class IndexBuffer;
class RenderTarget;
class ShaderVariation;
class InputLayout;
class Texture;

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

	//Rasterizer State
	void SetFillMode(const FillMode& fillMode);
	void SetCullMode(const CullMode& cullMode);

	//Blend State
	void SetBlendMode(const BlendMode& blendMode, const bool alphaToCoverage);
	void SetColorWrite(const ColorWrite colorWriteMask = ColorWrite::ALL);

	//Depthstencil State
	void SetDepthEnabled(const bool enabled);
	void SetDepthTest(const CompareMode& comparison);
	void SetStencilTest(bool stencilEnabled, const CompareMode mode, const StencilOperation pass, const StencilOperation fail, const StencilOperation zFail, const unsigned int stencilRef, const unsigned char compareMask, const unsigned char writeMask);

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

	ID3D11Device* GetDevice() const { return m_pDevice.Get(); }
	ID3D11DeviceContext* GetDeviceContext() const { return m_pDeviceContext.Get(); }
	unsigned int GetMultisampleQuality(const DXGI_FORMAT format, const unsigned int sampleCount) const;

private:
	void SetPrimitiveType(const PrimitiveType type);

	bool RegisterWindowClass();
	bool MakeWindow(int windowWidth, int windowHeight);
	bool EnumerateAdapters();
	bool CreateDevice(const int windowWidth, const int windowHeight);
	bool UpdateSwapchain(const int windowWidth, const int windowHeight);

	void UpdateRasterizerState();
	void UpdateBlendState();
	void UpdateDepthStencilState();

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

	//RasterizerState
	ComPtr<ID3D11RasterizerState> m_pRasterizerState;
	FillMode m_FillMode = FillMode::SOLID;
	CullMode m_CullMode = CullMode::BACK;
	bool m_RasterizerStateDirty = true;

	//BlendState
	ComPtr<ID3D11BlendState> m_pBlendState;
	bool m_BlendStateDirty = true;
	BlendMode m_BlendMode = BlendMode::REPLACE;
	bool m_AlphaToCoverage = false;
	ColorWrite m_ColorWriteMask = ColorWrite::ALL;
	
	//Depthstencilstate
	ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
	bool m_DepthStencilStateDirty = true;
	bool m_DepthEnabled = true;
	CompareMode m_DepthCompareMode = CompareMode::LESSEQUAL;
	bool m_StencilTestEnabled = false;
	CompareMode m_StencilTestMode = CompareMode::ALWAYS;
	StencilOperation m_StencilTestPassOperation = StencilOperation::KEEP;
	StencilOperation m_StencilTestFailOperation = StencilOperation::KEEP;
	StencilOperation m_StencilTestZFailOperation = StencilOperation::KEEP;
	unsigned int m_StencilRef = 0;
	unsigned char m_StencilCompareMask;
	unsigned char m_StencilWriteMask;

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