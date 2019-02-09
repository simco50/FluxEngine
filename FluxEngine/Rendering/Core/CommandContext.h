#pragma once
class Graphics;
class PipelineState;
class RenderTarget;
class VertexBuffer;
class IndexBuffer;
class StructuredBuffer;
class ShaderVariation;
class Texture;
class GraphicsPipelineState;
class ComputePipelineState;

class CommandContext
{
public:
	CommandContext(Graphics* pGraphics, void* pCommandList);
	~CommandContext();

	PipelineState* GetPipelineState() const { return m_pPipelineState.get(); }

	bool SetShaderParameter(StringHash hash, const void* pData);
	bool SetShaderParameter(StringHash hash, const void* pData, int stride, int count);
	bool SetShaderParameter(StringHash hash, float value);
	bool SetShaderParameter(StringHash hash, int value);
	bool SetShaderParameter(StringHash hash, const Vector2& value);
	bool SetShaderParameter(StringHash hash, const Vector3& value);
	bool SetShaderParameter(StringHash hash, const Vector4& value);
	bool SetShaderParameter(StringHash hash, const Color& value);
	bool SetShaderParameter(StringHash hash, const Matrix& value);

protected:
	Graphics* m_pGraphics;
	void* m_pCommandList;
	std::unique_ptr<PipelineState> m_pPipelineState;

	//Debug data
	unsigned int m_BatchCount = 0;
	unsigned int m_PrimitiveCount = 0;
};

class GraphicsCommandContext : public CommandContext
{
public:
	GraphicsCommandContext(Graphics* pGraphics, void* pCommandList);

	void SetRenderTarget(int index, RenderTarget* pRenderTarget);
	void SetDepthStencil(RenderTarget* pRenderTarget);

	void SetVertexBuffer(VertexBuffer* pBuffer);
	void SetVertexBuffers(VertexBuffer** pBuffers, int bufferCount, unsigned int instanceOffset = 0);

	void SetIndexBuffer(IndexBuffer* pIndexBuffer);

	void SetViewport(const FloatRect& rect);
	void SetScissorRect(bool enabled, const IntRect& rect = IntRect::ZERO());

	void SetTexture(TextureSlot slot, Texture* pTexture);
	void SetStructuredBuffer(TextureSlot slot, const StructuredBuffer* pBuffer);

	void Draw(PrimitiveType type, int vertexStart, int vertexCount);
	void DrawIndexed(PrimitiveType type, int indexCount, int indexStart, int minVertex = 0);
	void DrawIndexedInstanced(PrimitiveType type, int indexCount, int indexStart, int instanceCount, int minVertex = 0, int instanceStart = 0);

	void Clear(ClearFlags clearFlags = ClearFlags::All, const Color& color = Color(0.15f, 0.15f, 0.15f, 1.0f), float depth = 1.0f, unsigned char stencil = 0);

	void FlushRenderTargetChanges(bool force);
	void FlushSRVChanges(bool force);

	GraphicsPipelineState* GetGraphicsPipelineState() const;

private:
	void PrepareDraw();

	FloatRect m_CurrentViewport = FloatRect(0, 0, 1, 1);
	IntRect m_CurrentScissorRect;
	bool m_ScissorRectDirty = false;

	IndexBuffer* m_pCurrentIndexBuffer = nullptr;
	std::array<VertexBuffer*, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentVertexBuffers = {};

	RenderTarget* m_pCurrentDepthStencil = nullptr;
	std::array<RenderTarget*, GraphicsConstants::MAX_RENDERTARGETS> m_CurrentRenderTargets = {};
};

class ComputeCommandContext : public CommandContext
{
public:
	ComputeCommandContext(Graphics* pGraphics, void* pCommandList);

	ComputePipelineState* GetComputePipelineState() const;
};