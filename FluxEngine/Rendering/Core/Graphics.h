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
class Shader;

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

	//Window
	void SetWindowTitle(const string& title);
	void SetWindowPosition(const Vector2& position);

	//Graphics
	void SetRenderTarget(RenderTarget* pRenderTarget);
	void SetRenderTargets(const vector<RenderTarget*>& pRenderTargets);

	void SetVertexBuffer(VertexBuffer* pBuffer);
	void SetVertexBuffers(const vector<VertexBuffer*>& pBuffers, unsigned int instanceOffset = 0);

	void SetIndexBuffer(IndexBuffer* pIndexBuffer);

	bool SetShader(const ShaderType type, ShaderVariation* pShader);

	void SetViewport(const FloatRect& rect, bool relative = false);
	void SetScissorRect(const bool enabled, const IntRect& rect = IntRect::ZERO());

	void SetTexture(const TextureSlot slot, Texture* pTexture);

	void Draw(const PrimitiveType type, const int vertexStart, const int vertexCount);
	void DrawIndexed(const PrimitiveType type, const int indexCount, const int indexStart, const int minVertex = 0);
	void DrawIndexedInstanced(const PrimitiveType type, const int indexCount, const int indexStart, const int instanceCount, const int minVertex = 0, const int instanceStart = 0);

	void Clear(const ClearFlags clearFlags = ClearFlags::All, const Color& color = Color(0.15f, 0.15f, 0.15f, 1.0f), const float depth = 1.0f, const unsigned char stencil = 0);
	
	ConstantBuffer* GetOrCreateConstantBuffer(unsigned int size, const ShaderType shaderType, unsigned int registerIndex);
	Shader* GetShader(const string filePath);

	void BeginFrame();
	void EndFrame();

	void TakeScreenshot();
	void OnPause(const bool paused);

	//Getters
	const HWND& GetWindow() const { return m_Hwnd; }
	int GetWindowWidth() const { return m_WindowWidth; }
	int GetWindowHeight() const { return m_WindowHeight; }

	RenderTarget* GetRenderTarget() const { return m_pDefaultRenderTarget.get(); }
	BlendState* GetBlendState() const { return m_pBlendState.get(); }
	RasterizerState* GetRasterizerState() const { return m_pRasterizerState.get(); }
	DepthStencilState* GetDepthStencilState() const { return m_pDepthStencilState.get(); }

	GraphicsImpl* GetImpl() const { return m_pImpl.get(); }

	void GetDebugInfo(unsigned int& batchCount, unsigned int& primitiveCount);

private:

	void PrepareDraw();

	bool RegisterWindowClass();
	bool MakeWindow(int windowWidth, int windowHeight);
	bool EnumerateAdapters();
	bool CreateDevice(const int windowWidth, const int windowHeight);
	bool UpdateSwapchain();

	static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	int m_WindowWidth = 1240;
	int m_WindowHeight = 720;
	HWND m_Hwnd = nullptr;
	HINSTANCE m_hInstance;
	WindowType m_WindowType = WindowType::WINDOWED;
	bool m_Resizable = true;
	bool m_Vsync = true;
	int m_Multisample = 1;
	int m_RefreshRate = 60;
	string m_WindowTitle;
	string m_WindowClassName = "wndClass";

	bool m_Paused = false;
	bool m_Maximized = false;
	bool m_Minimized = false;
	bool m_Resizing = false;

	unique_ptr<GraphicsImpl> m_pImpl;

	unique_ptr<RenderTarget> m_pDefaultRenderTarget;
	unique_ptr<BlendState> m_pBlendState;
	unique_ptr<RasterizerState> m_pRasterizerState;
	unique_ptr<DepthStencilState> m_pDepthStencilState;

	//Cache of constantbuffers with a search hash
	map<unsigned int, unique_ptr<ConstantBuffer>> m_ConstantBuffers;

	using ShaderConstantBuffers = array<void*, (unsigned int)ShaderParameterType::MAX>;
	array<ShaderConstantBuffers, GraphicsConstants::SHADER_TYPES> m_CurrentConstBuffers = {};

	array<ShaderVariation*, GraphicsConstants::SHADER_TYPES> m_CurrentShaders = {};
	map<string, unique_ptr<Shader>> m_Shaders;

	FloatRect m_CurrentViewport = FloatRect(0, 0, 1, 1);
	IntRect m_CurrentScissorRect;
	bool m_ScissorEnabled = false;
	bool m_ScissorRectDirty = false;

	IndexBuffer* m_pCurrentIndexBuffer = nullptr;
	array<VertexBuffer*, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentVertexBuffers = {};
	unsigned int m_FirstDirtyVertexBuffer = numeric_limits<unsigned int>::max();
	unsigned int m_LastDirtyVertexBuffer = 0;
	bool m_VertexBuffersDirty = false;

	//Textures
	bool m_TexturesDirty = false;
	unsigned int m_FirstDirtyTexture = numeric_limits<unsigned int>::max();
	unsigned int m_LastDirtyTexture = 0;

	//Debug data
	unsigned int m_BatchCount = 0;
	unsigned int m_PrimitiveCount = 0;
};