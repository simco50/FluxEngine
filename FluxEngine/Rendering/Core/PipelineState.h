#pragma once
class Graphics;
class ShaderVariation;
class VertexBuffer;
struct ShaderParameter;

class PipelineState
{
public:
	PipelineState(Graphics* pGraphics);
	virtual ~PipelineState();

	virtual void ClearShaders() = 0;

	bool SetParameter(const std::string& name, const void* pData);
	bool SetParameter(StringHash hash, const void* pData);
	bool SetParameter(StringHash hash, const void* pData, int size);

protected:
	virtual void LoadShaderParameters() = 0;
	void LoadShaderParametersForShader(ShaderVariation* pShader);

	Graphics* m_pGraphics = nullptr;
	using ShaderConstantBuffers = std::array<void*, (size_t)ShaderParameterType::MAX>;
	std::array<ShaderConstantBuffers, (size_t)ShaderType::MAX> m_CurrentConstBuffers = {};

	BitField<8, uint32> m_DirtyShaders;
	bool m_ShaderParametersDirty = true;
	std::map<StringHash, const ShaderParameter*> m_ShaderParameters;
};

class GraphicsPipelineStateImpl;

class GraphicsPipelineState : public PipelineState
{
public:
	GraphicsPipelineState(Graphics* pGraphics);
	~GraphicsPipelineState();

	void Finalize(bool& hasUpdated, VertexBuffer** pVertexBuffers, int count);
	void Apply(VertexBuffer** pVertexBuffers, int count);

	void SetPrimitiveType(PrimitiveType type);

	//BlendState
	void SetBlendMode(const BlendMode& blendMode, bool alphaToCoverage);
	void SetColorWrite(ColorWrite colorWriteMask = ColorWrite::ALL);

	//DepthStencilState
	unsigned int GetStencilRef() const { return m_StencilRef; }
	void SetDepthEnabled(bool enabled);
	void SetDepthWrite(bool enabled);
	void SetDepthTest(const CompareMode& comparison);
	void SetStencilTest(bool stencilEnabled, CompareMode mode, StencilOperation pass, StencilOperation fail, StencilOperation zFail, unsigned int stencilRef, unsigned char compareMask, unsigned char writeMask);

	//RasterizerState
	void SetScissorEnabled(bool enabled);
	void SetMultisampleEnabled(bool enabled);
	void SetFillMode(FillMode fillMode);
	void SetCullMode(CullMode cullMode);
	void SetLineAntialias(bool lineAntiAlias);

	//Shaders
	virtual void ClearShaders() override;
	void SetVertexShader(ShaderVariation* pShader);
	void SetPixelShader(ShaderVariation* pShader);
	void SetGeometryShader(ShaderVariation* pShader);
	void SetHullShader(ShaderVariation* pShader);
	void SetDomainShader(ShaderVariation* pShader);

	ShaderVariation* GetVertexShader() const { return m_pVertexShader; }
	ShaderVariation* GetPixelShader() const { return m_pPixelShader; }
	ShaderVariation* GetGeometryShader() const { return m_pGeometryShader; }
	ShaderVariation* GetHullShader() const { return m_pHullShader; }
	ShaderVariation* GetDomainShader() const { return m_pDomainShader; }

private:
	void ApplyShader(ShaderType type, ShaderVariation* pShader);
	virtual void LoadShaderParameters() override;
	std::unique_ptr<GraphicsPipelineStateImpl> m_pImpl;

	bool m_IsDirty = false;
	bool m_IsCreated = false;

	unsigned int m_StencilRef = 0;

	//Shaders
	ShaderVariation* m_pVertexShader = nullptr;
	ShaderVariation* m_pPixelShader = nullptr;
	ShaderVariation* m_pGeometryShader = nullptr;
	ShaderVariation* m_pHullShader = nullptr;
	ShaderVariation* m_pDomainShader = nullptr;
};

class ComputePipelineState : public PipelineState
{
public:
	ComputePipelineState(Graphics* pGraphics);
	~ComputePipelineState();
	void Finalize(bool& hasUpdated);
	
	//Shaders
	virtual void ClearShaders() override;
	void SetComputeShader(ShaderVariation* pShader);

	ShaderVariation* GetComputeShader() const { return m_pComputeShader; }

private:
	void ApplyShader(ShaderType type, ShaderVariation* pShader);
	virtual void LoadShaderParameters() override;
	ShaderVariation* m_pComputeShader = nullptr;
};