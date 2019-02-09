#pragma once
class Graphics;

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
	~PipelineState();

	const PipelineStateData& GetData() const;
	void Finalize(bool& hasUpdated);

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

private:
	PipelineStateData m_Data;
	Graphics* m_pGraphics;
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
};