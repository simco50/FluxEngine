#pragma once
#include "Core\Subsystem.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"

class VertexBuffer;
class IndexBuffer;
class RenderTarget;
class ShaderVariation;
class InputLayout;
class Texture;
class ConstantBuffer;
class Shader;
class ShaderProgram;
class Texture2D;
class StructuredBuffer;
class GraphicsImpl;
struct SDL_Window;

struct AdapterInfo
{
	std::string Name;
	size_t VideoMemory = 0;
	size_t SystemMemory = 0;
	unsigned int DeviceId = 0;
	unsigned int VendorId = 0;
};

class Graphics : public Subsystem
{
	FLUX_OBJECT(Graphics, Subsystem)

public:
	explicit Graphics(Context* pContext);
	~Graphics();

	DELETE_COPY(Graphics)

	bool SetMode(
		const std::string& windowTitle,
		int width,
		int height,
		WindowType windowType,
		bool resizable,
		bool vsync,
		int multiSample,
		int refreshRate);

	void SetRenderTarget(int index, RenderTarget* pRenderTarget);
	void SetDepthStencil(RenderTarget* pRenderTarget);
	void SetDepthOnly(bool enable);

	void SetVertexBuffer(VertexBuffer* pBuffer);
	void SetVertexBuffers(VertexBuffer** pBuffers, int bufferCount, unsigned int instanceOffset = 0);

	void SetIndexBuffer(IndexBuffer* pIndexBuffer);

	void InvalidateShaders();
	bool SetShader(ShaderType type, ShaderVariation* pShader);

	void SetViewport(const FloatRect& rect);
	void SetScissorRect(bool enabled, const IntRect& rect = IntRect::ZERO());

	void SetTexture(TextureSlot slot, Texture* pTexture);
	void SetStructuredBuffer(TextureSlot slot, const StructuredBuffer* pBuffer);

	void Draw(PrimitiveType type, int vertexStart, int vertexCount);
	void DrawIndexed(PrimitiveType type, int indexCount, int indexStart, int minVertex = 0);
	void DrawIndexedInstanced(PrimitiveType type, int indexCount, int indexStart, int instanceCount, int minVertex = 0, int instanceStart = 0);

	void Clear(ClearFlags clearFlags = ClearFlags::All, const Color& color = Color(0.15f, 0.15f, 0.15f, 1.0f), float depth = 1.0f, unsigned char stencil = 0);

	ConstantBuffer* GetOrCreateConstantBuffer(unsigned int index, unsigned int size);
	Shader* GetShader(const std::string& filePath);
	ShaderVariation* GetShader(const std::string& filePath, ShaderType type, const std::string& defines = "");

	bool UsingTessellation() const;

	bool SetShaderParameter(StringHash hash, const void* pData);
	bool SetShaderParameter(StringHash hash, const void* pData, int stride, int count);
	bool SetShaderParameter(StringHash hash, float value);
	bool SetShaderParameter(StringHash hash, int value);
	bool SetShaderParameter(StringHash hash, const Vector2& value);
	bool SetShaderParameter(StringHash hash, const Vector3& value);
	bool SetShaderParameter(StringHash hash, const Vector4& value);
	bool SetShaderParameter(StringHash hash, const Color& value);
	bool SetShaderParameter(StringHash hash, const Matrix& value);

	void OnResize(int width, int height);
	void BeginFrame();
	void EndFrame();

	void TakeScreenshot();
	void TakeScreenshot(OutputStream& outputStream);

	//Getters
	const std::string& GetWindowTitle() const { return m_WindowTitle; }
	HWND GetWindow() const;
	SDL_Window* GetSDLWindow() const { return m_pWindow; }
	int GetWindowWidth() const { return m_WindowWidth; }
	int GetWindowHeight() const { return m_WindowHeight; }
	int GetMultisample() const { return m_Multisample; }

	bool GetAdapterInfo(AdapterInfo& adapterInfo);

	RenderTarget* GetRenderTarget() const;
	RenderTarget* GetDepthStencil() const { return m_pCurrentDepthStencil; }
	BlendState* GetBlendState() { return &m_BlendState; }
	RasterizerState* GetRasterizerState() { return &m_RasterizerState; }
	DepthStencilState* GetDepthStencilState() { return &m_DepthStencilState; }

	GraphicsImpl* GetImpl() const { return m_pImpl.get(); }

	void GetDebugInfo(unsigned int& batchCount, unsigned int& primitiveCount);

	void FlushRenderTargetChanges(bool force);
	void FlushSRVChanges(bool force);

private:
	void PrepareDraw();

	bool OpenWindow();
	bool EnumerateAdapters();
	bool CreateDevice(int windowWidth, int windowHeight);
	void UpdateSwapchain(int windowWidth, int windowHeight);

	void UpdateShaderProgram();
	void UpdateShaders();

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

	BlendState m_BlendState;
	RasterizerState m_RasterizerState;
	DepthStencilState m_DepthStencilState;

	//All ConstantBuffers
	std::map<size_t, std::unique_ptr<ConstantBuffer>> m_ConstantBuffers;
	using ShaderConstantBuffers = std::array<void*, (size_t)ShaderParameterType::MAX>;
	std::array<ShaderConstantBuffers, (size_t)ShaderType::MAX> m_CurrentConstBuffers = {};
	std::array<ShaderVariation*, (size_t)ShaderType::MAX> m_CurrentShaders = {};
	static std::string m_ShaderExtension;

	FloatRect m_CurrentViewport = FloatRect(0, 0, 1, 1);
	IntRect m_CurrentScissorRect;
	bool m_ScissorEnabled = false;
	bool m_ScissorRectDirty = false;
	bool m_RenderDepthOnly = false;

	IndexBuffer* m_pCurrentIndexBuffer = nullptr;
	std::array<VertexBuffer*, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentVertexBuffers = {};
	std::array<RenderTarget*, GraphicsConstants::MAX_RENDERTARGETS> m_CurrentRenderTargets = {};
	RenderTarget* m_pCurrentDepthStencil = nullptr;

	std::unique_ptr<Texture2D> m_pDefaultRenderTarget;
	std::unique_ptr<Texture2D> m_pDefaultDepthStencil;
	BitField<(int)ShaderType::MAX> m_DirtyShaders;

	//Debug data
	unsigned int m_BatchCount = 0;
	unsigned int m_PrimitiveCount = 0;
};