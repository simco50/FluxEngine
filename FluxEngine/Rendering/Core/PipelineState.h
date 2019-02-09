#pragma once
class Graphics;
class ShaderVariation;
class VertexBuffer;
struct ShaderParameter;

#ifdef GRAPHICS_D3D11
struct PipelineStateData
{
	void* pRasterizerState = nullptr;
	void* pDepthStencilState = nullptr;
	void* pBlendState = nullptr;

};
#elif GRAPHICS_D3D12
using PipelineStateData = void*;
#endif

class PipelineState
{
public:
	PipelineState(Graphics* pGraphics);
	virtual ~PipelineState();
	const PipelineStateData& GetData() const;

	bool SetParameter(const std::string& name, const void* pData);
	bool SetParameter(StringHash hash, const void* pData);
	bool SetParameter(StringHash hash, const void* pData, int size);

protected:
	virtual void LoadShaderParameters() = 0;
	void ApplyShader(ShaderType type, ShaderVariation* pShader);

	Graphics* m_pGraphics = nullptr;
	PipelineStateData m_Data;
	using ShaderConstantBuffers = std::array<void*, (size_t)ShaderParameterType::MAX>;
	std::array<ShaderConstantBuffers, (size_t)ShaderType::MAX> m_CurrentConstBuffers = {};

	BitField<8, uint32> m_DirtyShaders;
	bool m_ShaderParametersDirty = true;
	void LoadShaderParametersForShader(ShaderVariation* pShader);
	std::map<StringHash, const ShaderParameter*> m_ShaderParameters;
};

class GraphicsPipelineState : public PipelineState
{
public:
	GraphicsPipelineState(Graphics* pGraphics);
	~GraphicsPipelineState();

	void Finalize(bool& hasUpdated);
	void Apply(VertexBuffer** pVertexBuffers, int count);

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

	void ApplyInputLayout(VertexBuffer** pVertexBuffers, int count);

private:
	virtual void LoadShaderParameters() override;

	bool m_IsDirty = false;
	bool m_IsCreated = false;

	//BlendState
	BlendMode m_BlendMode = BlendMode::REPLACE;
	bool m_AlphaToCoverage = false;
	ColorWrite m_ColorWriteMask = ColorWrite::ALL;

	//DepthStencilState
	bool m_DepthEnabled = true;
	bool m_DepthWrite = true;
	CompareMode m_DepthCompareMode = CompareMode::LESS;
	bool m_StencilTestEnabled = false;
	CompareMode m_StencilTestMode = CompareMode::ALWAYS;
	StencilOperation m_StencilTestPassOperation = StencilOperation::KEEP;
	StencilOperation m_StencilTestFailOperation = StencilOperation::KEEP;
	StencilOperation m_StencilTestZFailOperation = StencilOperation::KEEP;
	unsigned int m_StencilRef = 0;
	unsigned char m_StencilCompareMask = 0;
	unsigned char m_StencilWriteMask = 0;

	//RasterizerState
	bool m_LineAntiAlias = false;
	bool m_ScissorEnabled = false;
	bool m_MultisampleEnabled = true;
	FillMode m_FillMode = FillMode::SOLID;
	CullMode m_CullMode = CullMode::BACK;

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

	void SetComputeShader(ShaderVariation* pShader);

	ShaderVariation* GetComputeShader() const { return m_pComputeShader; }

private:
	ShaderVariation* m_pComputeShader = nullptr;
};