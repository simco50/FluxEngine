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

class GraphicsImpl;

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
	void SetVertexBuffers(const vector<VertexBuffer*>& pBuffers, unsigned int instanceOffset = 0);

	void SetIndexBuffer(IndexBuffer* pIndexBuffer);

	void SetShaders(ShaderVariation* pVertexShader, ShaderVariation* pPixelShader);

	void SetViewport(const FloatRect& rect, bool relative = false);
	void SetScissorRect(const bool enabled, const IntRect& rect = IntRect::ZERO());

	void SetTexture(const unsigned int index, Texture* pTexture);

	void Draw(const PrimitiveType type, const int vertexStart, const int vertexCount);
	void DrawIndexed(const PrimitiveType type, const int indexCount, const int indexStart, const int minVertex = 0);
	void DrawIndexedInstanced(const PrimitiveType type, const int indexCount, const int indexStart, const int instanceCount, const int minVertex = 0, const int instanceStart = 0);

	void Clear(const unsigned int flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, const XMFLOAT4& color = (XMFLOAT4)DirectX::Colors::CornflowerBlue, const float depth = 1.0f, const unsigned char stencil = 0);

	void PrepareDraw();
	void BeginFrame();
	void EndFrame();

	void OnPause(const bool paused);

	//Getters
	const HWND& GetWindow() const { return m_Hwnd; }
	int GetWindowWidth() const { return m_WindowWidth; }
	int GetWindowHeight() const { return m_WindowHeight; }

	RenderTarget* GetRenderTarget() const { return m_pDefaultRenderTarget.get(); }

	BlendState* GetBlendState() const { return m_pBlendState.get(); }
	RasterizerState* GetRasterizerState() const { return m_pRasterizerState.get(); }
	DepthStencilState* GetDepthStencilState() const { return m_pDepthStencilState.get(); }

	ConstantBuffer* GetOrCreateConstantBuffer(unsigned int size, const ShaderType shaderType, unsigned int registerIndex);

	unsigned int GetMultisampleQuality(const DXGI_FORMAT format, const unsigned int sampleCount) const;

	GraphicsImpl* GetImpl() const { return m_pImpl.get(); }

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

	unique_ptr<GraphicsImpl> m_pImpl;

	unique_ptr<RenderTarget> m_pDefaultRenderTarget;
	unique_ptr<BlendState> m_pBlendState;
	unique_ptr<RasterizerState> m_pRasterizerState;
	unique_ptr<DepthStencilState> m_pDepthStencilState;

	map<unsigned int, unique_ptr<ConstantBuffer>> m_ConstantBuffers;
	array<array<void*, (unsigned int)ShaderParameterType::MAX>, (unsigned int)ShaderType::MAX> m_CurrentConstBuffers = {};

	//Clip rect
	IntRect m_CurrentScissorRect;
	bool m_ScissorEnabled = false;
	bool m_ScissorRectDirty = true;

	IndexBuffer* m_pCurrentIndexBuffer = nullptr;
	array<VertexBuffer*, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentVertexBuffers = {};
	unsigned int m_FirstDirtyVertexBuffer = numeric_limits<unsigned int>::max();
	unsigned int m_LastDirtyVertexBuffer = 0;
	bool m_VertexBuffersDirty = false;
	PrimitiveType m_CurrentPrimitiveType = PrimitiveType::UNDEFINED;
	FloatRect m_CurrentViewport = FloatRect(0, 0, 1, 1);

	ShaderVariation* m_pCurrentVertexShader = nullptr;
	ShaderVariation* m_pCurrentPixelShader = nullptr;

	bool m_TexturesDirty = false;
	unsigned int m_FirstDirtyTexture = numeric_limits<unsigned int>::max();
	unsigned int m_LastDirtyTexture = 0;

};