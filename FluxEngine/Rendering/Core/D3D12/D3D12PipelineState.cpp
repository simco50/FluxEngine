#include "FluxEngine.h"
#include "../PipelineState.h"
#include "../Graphics.h"
#include "D3D12GraphicsImpl.h"
#include "../ShaderVariation.h"
#include "D3D12Helpers.h"
#include "../VertexBuffer.h"
#include "../D3DCommon/D3DCommon.h"
#include "../Texture2D.h"

class GraphicsPipelineStateImpl
{
	friend class GraphicsPipelineState;
public:
	GraphicsPipelineStateImpl()
	{
		Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		Desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		Desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		Desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		Desc.InputLayout.NumElements = 0;
		Desc.InputLayout.pInputElementDescs = nullptr;
		Desc.NodeMask = 0;
		Desc.NumRenderTargets = 1;
		Desc.pRootSignature = nullptr;
		Desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		Desc.SampleDesc.Count = 1;
		Desc.SampleDesc.Quality = 0;
		Desc.SampleMask = UINT_MAX;
	}

private:
	ID3D12PipelineState* pPipelineState = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc = {};
	BlendMode BlendMode;
	ColorWrite ColorWriteMask;
};

GraphicsPipelineState::GraphicsPipelineState(Graphics* pGraphics)
	: PipelineState(pGraphics), m_pImpl(std::make_unique<GraphicsPipelineStateImpl>())
{
}

GraphicsPipelineState::~GraphicsPipelineState()
{
}

void GraphicsPipelineState::Finalize(bool& hasUpdated, VertexBuffer** pVertexBuffers, int count)
{
	AUTOPROFILE(GraphicsPipelineState_Finalize);

	hasUpdated = false;

	if (m_IsDirty || m_IsCreated == false)
	{
		GraphicsImpl* pImpl = m_pGraphics->GetImpl();

		//InputLayout
		{
			AUTOPROFILE(GraphicsPipelineState_InputLayout_Create);

			checkf(m_pVertexShader, "[GraphicsCommandContext] No vertex shader set");

			m_pImpl->InputLayout.clear();
			for (int i = 0; i < count; ++i)
			{
				if (pVertexBuffers[i] == nullptr)
				{
					continue;
				}

				for (const VertexElement& e : pVertexBuffers[i]->GetElements())
				{
					D3D12_INPUT_ELEMENT_DESC desc;
					desc.SemanticName = VertexElement::GetSemanticOfType(e.Semantic);
					desc.Format = D3DCommon::GetFormatOfType(e.Type);
					desc.AlignedByteOffset = e.Offset;
					desc.InputSlotClass = e.PerInstance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
					desc.InputSlot = i;
					desc.InstanceDataStepRate = e.PerInstance ? 1 : 0;
					desc.SemanticIndex = e.Index;

					m_pImpl->InputLayout.push_back(desc);
				}
			}
			m_pImpl->Desc.InputLayout.NumElements = (uint32)m_pImpl->InputLayout.size();
			m_pImpl->Desc.InputLayout.pInputElementDescs = m_pImpl->InputLayout.data();

			//Create Pipeline state
			ComPtr<ID3D12PipelineState> pPipelineState;
			HR(pImpl->GetDevice()->CreateGraphicsPipelineState(&m_pImpl->Desc, IID_PPV_ARGS(pPipelineState.GetAddressOf())));
			pImpl->m_PipelineStateCache.push_back(std::move(pPipelineState));
			m_pImpl->pPipelineState = pImpl->m_PipelineStateCache.back().Get();
		}
		
		m_IsDirty = false;
		m_IsCreated = true;
		hasUpdated = true;
	}
}

void GraphicsPipelineState::ApplyShader(ShaderType type, ShaderVariation* pShader)
{
	AUTOPROFILE_DESC(GraphicsPipelineState_ApplyShader, pShader ? pShader->GetName() : "None");
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc = m_pImpl->Desc;

	switch (type)
	{
	case ShaderType::VertexShader:
		desc.VS.pShaderBytecode = pShader->GetByteCode().data();
		desc.VS.BytecodeLength = pShader->GetByteCode().size();
		break;
	case ShaderType::PixelShader:
		desc.PS.pShaderBytecode = pShader->GetByteCode().data();
		desc.PS.BytecodeLength = pShader->GetByteCode().size();
		break;
	case ShaderType::GeometryShader:
		desc.GS.pShaderBytecode = pShader->GetByteCode().data();
		desc.GS.BytecodeLength = pShader->GetByteCode().size();
		break;
	case ShaderType::DomainShader:
		desc.DS.pShaderBytecode = pShader->GetByteCode().data();
		desc.DS.BytecodeLength = pShader->GetByteCode().size();
		break;
	case ShaderType::HullShader:
		desc.HS.pShaderBytecode = pShader->GetByteCode().data();
		desc.HS.BytecodeLength = pShader->GetByteCode().size();
		break;
	default:
		FLUX_LOG(Error, "[Graphics::ApplyShader] > Shader type not implemented");
		return;
	}
}

void GraphicsPipelineState::Apply(VertexBuffer** pVertexBuffers, int count)
{
	AUTOPROFILE(GraphicsPipelineState_Apply);

	if (m_DirtyShaders.GetBit((int)ShaderType::VertexShader))
	{
		ApplyShader(ShaderType::VertexShader, m_pVertexShader);
	}
	if (m_DirtyShaders.GetBit((int)ShaderType::PixelShader))
	{
		ApplyShader(ShaderType::PixelShader, m_pPixelShader);
	}
	if (m_DirtyShaders.GetBit((int)ShaderType::GeometryShader))
	{
		ApplyShader(ShaderType::GeometryShader, m_pGeometryShader);
	}
	if (m_DirtyShaders.GetBit((int)ShaderType::HullShader))
	{
		ApplyShader(ShaderType::HullShader, m_pHullShader);
	}
	if (m_DirtyShaders.GetBit((int)ShaderType::DomainShader))
	{
		ApplyShader(ShaderType::DomainShader, m_pDomainShader);
	}
	m_DirtyShaders.ClearAll();

	bool pipelineStateUpdated = false;
	Finalize(pipelineStateUpdated, pVertexBuffers, count);
}

void GraphicsPipelineState::SetPrimitiveType(PrimitiveType type)
{
	m_pImpl->Desc.PrimitiveTopologyType = D3D12Helpers::GetPrimitiveType(type);
	m_IsDirty = true;
}

void GraphicsPipelineState::OnRenderTargetsSet(Texture2D** pRenderTargets, int count, Texture2D* pDepthStencil)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc = m_pImpl->Desc;
	if (desc.NumRenderTargets != (uint32)count)
	{
		desc.NumRenderTargets = (uint32)count;
		m_IsDirty = true;
	}
	for (int i = 0; i < count; ++i)
	{
		DXGI_FORMAT format = (DXGI_FORMAT)pRenderTargets[i]->GetFormat();
		if (format != desc.RTVFormats[i])
		{
			desc.RTVFormats[i] = desc.RTVFormats[i];
			m_IsDirty = true;
		}
	}
	DXGI_FORMAT depthFormat = (DXGI_FORMAT)pDepthStencil->GetFormat();
	if (desc.DSVFormat != depthFormat)
	{
		desc.DSVFormat = depthFormat;
		m_IsDirty = true;
	}
}

void GraphicsPipelineState::SetBlendMode(const BlendMode& blendMode, const bool alphaToCoverage)
{
	if (blendMode != m_pImpl->BlendMode || alphaToCoverage != (bool)m_pImpl->Desc.BlendState.AlphaToCoverageEnable)
	{
		m_pImpl->BlendMode = blendMode;
		m_pImpl->Desc.BlendState.RenderTarget[0] = D3D12Helpers::D3D12RenderTargetBlendDesc(blendMode, (unsigned char)m_pImpl->ColorWriteMask);
		m_pImpl->Desc.BlendState.AlphaToCoverageEnable = alphaToCoverage;

		m_IsDirty = true;
	}
}

void GraphicsPipelineState::SetColorWrite(const ColorWrite colorWriteMask /*= ColorWrite::ALL*/)
{
	if (m_pImpl->ColorWriteMask != colorWriteMask)
	{
		m_pImpl->ColorWriteMask = colorWriteMask;
		m_pImpl->Desc.BlendState.RenderTarget[0] = D3D12Helpers::D3D12RenderTargetBlendDesc(m_pImpl->BlendMode, (unsigned char)colorWriteMask);
		m_IsDirty = true;
	}
}

void GraphicsPipelineState::SetDepthEnabled(const bool enabled)
{
	if (enabled != (bool)m_pImpl->Desc.DepthStencilState.DepthEnable)
	{
		m_pImpl->Desc.DepthStencilState.DepthEnable = enabled;
		m_IsDirty = true;
	}
}

void GraphicsPipelineState::SetDepthWrite(const bool enabled)
{
	D3D12_DEPTH_WRITE_MASK mask = enabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	if (mask != m_pImpl->Desc.DepthStencilState.DepthWriteMask)
	{
		m_pImpl->Desc.DepthStencilState.DepthWriteMask = mask;
		m_IsDirty = true;
	}
}

void GraphicsPipelineState::SetDepthTest(const CompareMode& comparison)
{
	D3D12_COMPARISON_FUNC func = D3D12Helpers::D3D12ComparisonFunction(comparison);
	if (func != m_pImpl->Desc.DepthStencilState.DepthFunc)
	{
		m_pImpl->Desc.DepthStencilState.DepthFunc = func;
		m_IsDirty = true;
	}
}

void GraphicsPipelineState::SetStencilTest(bool stencilEnabled, const CompareMode mode, const StencilOperation pass, const StencilOperation fail, const StencilOperation zFail, const unsigned int stencilRef, const unsigned char compareMask, unsigned char writeMask)
{
	m_StencilRef = stencilRef;
	D3D12_DEPTH_STENCIL_DESC& desc = m_pImpl->Desc.DepthStencilState;

	if (stencilEnabled != (bool)desc.StencilEnable)
	{
		desc.StencilEnable = stencilEnabled;
		m_IsDirty = true;
	}

	D3D12_COMPARISON_FUNC testMode = D3D12Helpers::D3D12ComparisonFunction(mode);
	if (testMode != desc.FrontFace.StencilFunc)
	{
		desc.FrontFace.StencilFunc = testMode;
		desc.BackFace.StencilFunc = testMode;
		m_IsDirty = true;
	}

	D3D12_STENCIL_OP passOp = D3D12Helpers::D3D12StencilOperation(pass);
	if (passOp != desc.FrontFace.StencilPassOp)
	{
		desc.FrontFace.StencilPassOp = passOp;
		desc.BackFace.StencilPassOp = passOp;
		m_IsDirty = true;
	}

	D3D12_STENCIL_OP failOp = D3D12Helpers::D3D12StencilOperation(fail);
	if (failOp != desc.FrontFace.StencilFailOp)
	{
		desc.FrontFace.StencilFailOp = passOp;
		desc.BackFace.StencilFailOp = passOp;
		m_IsDirty = true;
	}

	D3D12_STENCIL_OP zFailOp = D3D12Helpers::D3D12StencilOperation(zFail);
	if (zFailOp != desc.FrontFace.StencilDepthFailOp)
	{
		desc.FrontFace.StencilDepthFailOp = zFailOp;
		desc.BackFace.StencilDepthFailOp = zFailOp;
		m_IsDirty = true;
	}
	if (compareMask != desc.StencilReadMask)
	{
		desc.StencilReadMask = compareMask;
		m_IsDirty = true;
	}
	if (writeMask != desc.StencilWriteMask)
	{
		desc.StencilWriteMask = writeMask;
		m_IsDirty = true;
	}
}

void GraphicsPipelineState::SetFillMode(FillMode fillMode)
{
	D3D12_FILL_MODE f = D3D12Helpers::D3D12FillMode(fillMode);
	if (f != m_pImpl->Desc.RasterizerState.FillMode)
	{
		m_pImpl->Desc.RasterizerState.FillMode = f;
		m_IsDirty = true;
	}
}

void GraphicsPipelineState::SetCullMode(CullMode cullMode)
{
	D3D12_CULL_MODE c = D3D12Helpers::D3D12CullMode(cullMode);
	if (c != m_pImpl->Desc.RasterizerState.CullMode)
	{
		m_pImpl->Desc.RasterizerState.CullMode = c;
		m_IsDirty = true;
	}
}

void GraphicsPipelineState::SetLineAntialias(bool lineAntiAlias)
{
	if (lineAntiAlias != (bool)m_pImpl->Desc.RasterizerState.AntialiasedLineEnable)
	{
		m_pImpl->Desc.RasterizerState.AntialiasedLineEnable = lineAntiAlias;
		m_IsDirty = true;
	}
}

void GraphicsPipelineState::SetScissorEnabled(bool /*enabled*/)
{
	//Nop in D3D12
}

void GraphicsPipelineState::SetMultisampleEnabled(bool enabled)
{
	if (enabled != (bool)m_pImpl->Desc.RasterizerState.MultisampleEnable)
	{
		m_pImpl->Desc.RasterizerState.MultisampleEnable = enabled;
		m_IsDirty = true;
	}
}

/////////Compute Pipeline State
////////////////////////////////////////////

void ComputePipelineState::Finalize(bool& hasUpdated)
{
	hasUpdated = true;
}

void ComputePipelineState::ApplyShader(ShaderType type, ShaderVariation* pShader)
{
	AUTOPROFILE_DESC(GraphicsPipelineState_ApplyShader, pShader ? pShader->GetName() : "None");
}