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
class ShaderProgram;
class Window;

class GraphicsImpl;

class Graphics
{
public:
	Graphics(Window* pWindow);
	~Graphics();

	DELETE_COPY(Graphics)

	bool SetMode(
		const bool vsync,
		const int multiSample,
		const int refreshRate);

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
	
	ConstantBuffer* GetOrCreateConstantBuffer(const std::string& name, unsigned int size);
	Shader* GetShader(const string filePath);
	bool SetShaderParameter(const std::string& name, const void* pData);

	void BeginFrame();
	void EndFrame();

	void TakeScreenshot();

	//Getters
	Window* GetWindow() const { return m_pWindow; }
	int GetWindowWidth() const { return m_Width; }
	int GetWindowHeight() const { return m_Height; }

	RenderTarget* GetRenderTarget() const { return m_pDefaultRenderTarget.get(); }
	BlendState* GetBlendState() const { return m_pBlendState.get(); }
	RasterizerState* GetRasterizerState() const { return m_pRasterizerState.get(); }
	DepthStencilState* GetDepthStencilState() const { return m_pDepthStencilState.get(); }

	GraphicsImpl* GetImpl() const { return m_pImpl.get(); }

	void GetDebugInfo(unsigned int& batchCount, unsigned int& primitiveCount);
private:
	void PrepareDraw();

	bool EnumerateAdapters();
	bool CreateDevice(const int windowWidth, const int windowHeight);
	void UpdateSwapchain(int windowWidth, int windowHeight);

	Window* m_pWindow;
	DelegateHandle m_WindowSizeChangedHandle;
	int m_Width = 0;
	int m_Height = 0;
	bool m_Vsync = true;
	int m_Multisample = 1;
	int m_RefreshRate = 60;

	unique_ptr<GraphicsImpl> m_pImpl;

	unique_ptr<RenderTarget> m_pDefaultRenderTarget;
	unique_ptr<BlendState> m_pBlendState;
	unique_ptr<RasterizerState> m_pRasterizerState;
	unique_ptr<DepthStencilState> m_pDepthStencilState;

	//All ConstantBuffers
	map<std::string, unique_ptr<ConstantBuffer>> m_ConstantBuffers;
	//Reference to all current ConstantBuffers
	using ShaderConstantBuffers = array<void*, (size_t)ShaderParameterType::MAX>;
	array<ShaderConstantBuffers, (size_t)ShaderType::MAX> m_CurrentConstBuffers = {};
	//References to all current shaders
	array<ShaderVariation*, (size_t)ShaderType::MAX> m_CurrentShaders = {};
	//All shaders
	map<string, unique_ptr<Shader>> m_Shaders;

	ShaderProgram* m_pCurrentShaderProgram = nullptr;
	map<unsigned int, unique_ptr<ShaderProgram>> m_ShaderPrograms;
	bool m_ShaderProgramDirty = false;

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