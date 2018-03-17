#pragma once
#include "GraphicsDefines.h"
#include "Core\Subsystem.h"

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
class ShaderProgram;

class GraphicsImpl;

class Graphics : public Subsystem
{
	FLUX_OBJECT(Graphics, Subsystem)

public:
	Graphics(Context* pContext);
	~Graphics();

	DELETE_COPY(Graphics)

	bool SetMode(
		const std::string& windowTitle,
		const int width,
		const int height,
		WindowType windowType,
		const bool resizable,
		const bool vsync,
		const int multiSample,
		const int refreshRate);

	void SetRenderTarget(const int index, RenderTarget* pRenderTarget);

	void SetVertexBuffer(VertexBuffer* pBuffer);
	void SetVertexBuffers(const std::vector<VertexBuffer*>& pBuffers, unsigned int instanceOffset = 0);

	void SetIndexBuffer(IndexBuffer* pIndexBuffer);

	bool SetShader(const ShaderType type, ShaderVariation* pShader);

	void SetViewport(const FloatRect& rect, bool relative = false);
	void SetScissorRect(const bool enabled, const IntRect& rect = IntRect::ZERO());

	void SetTexture(const TextureSlot slot, Texture* pTexture);

	void Draw(const PrimitiveType type, const int vertexStart, const int vertexCount);
	void DrawIndexed(const PrimitiveType type, const int indexCount, const int indexStart, const int minVertex = 0);
	void DrawIndexedInstanced(const PrimitiveType type, const int indexCount, const int indexStart, const int instanceCount, const int minVertex = 0, const int instanceStart = 0);

	void Clear(const ClearFlags clearFlags = ClearFlags::All, const Color& color = Color(0.15f, 0.15f, 0.15f, 1.0f), const float depth = 1.0f, const unsigned char stencil = 0);
	
	ConstantBuffer* GetOrCreateConstantBuffer(const std::string& name, unsigned int size);
	Shader* GetShader(const std::string filePath);
	ShaderVariation* GetShader(const std::string filePath, const ShaderType type, const std::string& defines = "");
	bool SetShaderParameter(const std::string& name, const void* pData);

	void OnResize(const int width, const int height);
	void BeginFrame();
	void EndFrame();

	void TakeScreenshot();

	//Getters
	HWND GetWindow() const;
	SDL_Window* GetSDLWindow() const { return m_pWindow; }
	int GetWindowWidth() const { return m_WindowWidth; }
	int GetWindowHeight() const { return m_WindowHeight; }
	int GetMultisample() const { return m_Multisample; }

	RenderTarget* GetRenderTarget() const { return m_pDefaultRenderTarget.get(); }
	BlendState* GetBlendState() const { return m_pBlendState.get(); }
	RasterizerState* GetRasterizerState() const { return m_pRasterizerState.get(); }
	DepthStencilState* GetDepthStencilState() const { return m_pDepthStencilState.get(); }

	GraphicsImpl* GetImpl() const { return m_pImpl.get(); }

	void GetDebugInfo(unsigned int& batchCount, unsigned int& primitiveCount);

	void PrepareDraw();
private:

	bool OpenWindow();
	bool EnumerateAdapters();
	bool CreateDevice(const int windowWidth, const int windowHeight);
	void UpdateSwapchain(int windowWidth, int windowHeight);

	DelegateHandle m_WindowSizeChangedHandle;
	SDL_Window* m_pWindow = nullptr;
	int m_WindowWidth = 0;
	int m_WindowHeight = 0;
	bool m_Resizable = true;
	WindowType m_WindowType;
	std::string m_WindowTitle;
	bool m_Vsync = true;
	int m_Multisample = 1;
	int m_RefreshRate = 60;

	std::unique_ptr<GraphicsImpl> m_pImpl;

	std::unique_ptr<RenderTarget> m_pDefaultRenderTarget;
	std::unique_ptr<BlendState> m_pBlendState;
	std::unique_ptr<RasterizerState> m_pRasterizerState;
	std::unique_ptr<DepthStencilState> m_pDepthStencilState;

	//All ConstantBuffers
	std::map<std::string, std::unique_ptr<ConstantBuffer>> m_ConstantBuffers;
	using ShaderConstantBuffers = std::array<void*, (size_t)ShaderParameterType::MAX>;
	std::array<ShaderConstantBuffers, (size_t)ShaderType::MAX> m_CurrentConstBuffers = {};
	std::array<ShaderVariation*, (size_t)ShaderType::MAX> m_CurrentShaders = {};
	static std::string m_ShaderExtension;

	FloatRect m_CurrentViewport = FloatRect(0, 0, 1, 1);
	IntRect m_CurrentScissorRect;
	bool m_ScissorEnabled = false;
	bool m_ScissorRectDirty = false;

	IndexBuffer* m_pCurrentIndexBuffer = nullptr;
	std::array<VertexBuffer*, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentVertexBuffers = {};
	std::array<RenderTarget*, GraphicsConstants::MAX_RENDERTARGETS> m_CurrentRenderTargets;

	//Debug data
	unsigned int m_BatchCount = 0;
	unsigned int m_PrimitiveCount = 0;
};