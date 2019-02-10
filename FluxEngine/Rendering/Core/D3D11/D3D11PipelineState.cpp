#include "FluxEngine.h"
#include "../PipelineState.h"
#include "../Graphics.h"
#include "../ShaderVariation.h"
#include "../ConstantBuffer.h"
#include "../VertexBuffer.h"
#include "../D3DCommon/D3DDefines.h"
#include "D3D11GraphicsImpl.h"
#include "D3D11Helpers.h"

/////////Pipeline State
////////////////////////////////////////////

/////////Graphics Pipeline State
////////////////////////////////////////////

class GraphicsPipelineStateImpl
{
	friend class GraphicsPipelineState;
public:
	GraphicsPipelineStateImpl()
	{
		BlendStateDesc.AlphaToCoverageEnable = false;
		BlendStateDesc.IndependentBlendEnable = false;
		BlendStateDesc.RenderTarget[0] = D3D11RenderTargetBlendDesc(BlendMode, (unsigned char)ColorWriteMask);

		RasterizerStateDesc.AntialiasedLineEnable = false;
		RasterizerStateDesc.CullMode = D3D11_CULL_FRONT;
		RasterizerStateDesc.DepthBias = 0;
		RasterizerStateDesc.DepthBiasClamp = 1.0f;
		RasterizerStateDesc.DepthClipEnable = true;
		RasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		RasterizerStateDesc.FrontCounterClockwise = false;
		RasterizerStateDesc.MultisampleEnable = false;
		RasterizerStateDesc.ScissorEnable = false;
		RasterizerStateDesc.SlopeScaledDepthBias = 0.0f;

		DepthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
		DepthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
		DepthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		DepthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
		DepthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
		DepthStencilStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
		DepthStencilStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		DepthStencilStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
		DepthStencilStateDesc.DepthEnable = true;
		DepthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		DepthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		DepthStencilStateDesc.StencilEnable = false;
		DepthStencilStateDesc.StencilReadMask = 0;
		DepthStencilStateDesc.StencilWriteMask = 0;
	}
	~GraphicsPipelineStateImpl() = default;

private:
	ID3D11BlendState* pBlendState = nullptr;
	ID3D11DepthStencilState* pDepthStencilState = nullptr;
	ID3D11RasterizerState* pRasterizerState = nullptr;
	ID3D11InputLayout* pInputLayout = nullptr;

	D3D11_BLEND_DESC BlendStateDesc;
	D3D11_RASTERIZER_DESC RasterizerStateDesc;
	D3D11_DEPTH_STENCIL_DESC DepthStencilStateDesc;

	BlendMode BlendMode = BlendMode::REPLACE;
	ColorWrite ColorWriteMask = ColorWrite::ALL;
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

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	m_IsDirty = false;
	hasUpdated = false;

	if(pImpl->m_DepthStencilStateDirty || m_IsCreated == false)
	{
		//Depth Stencil State
		hasUpdated = true;
		pImpl->m_DepthStencilStateDirty = false;
		unsigned int stateHash =
			(unsigned char)m_pImpl->DepthStencilStateDesc.DepthEnable<< 0
			| (unsigned char)m_pImpl->DepthStencilStateDesc.DepthWriteMask << 1
			| (unsigned char)m_pImpl->DepthStencilStateDesc.DepthFunc << 2
			| (unsigned char)m_pImpl->DepthStencilStateDesc.StencilEnable << 6
			| (unsigned char)m_pImpl->DepthStencilStateDesc.FrontFace.StencilFunc << 7
			| (unsigned char)m_pImpl->DepthStencilStateDesc.FrontFace.StencilPassOp << 11
			| (unsigned char)m_pImpl->DepthStencilStateDesc.FrontFace.StencilFailOp << 15
			| (unsigned char)m_pImpl->DepthStencilStateDesc.FrontFace.StencilDepthFailOp << 19
			| (unsigned char)m_pImpl->DepthStencilStateDesc.StencilReadMask << 23
			| (unsigned char)m_pImpl->DepthStencilStateDesc.StencilWriteMask << 27;

		auto state = pImpl->m_DepthStencilStates.find(stateHash);
		if (state != pImpl->m_DepthStencilStates.end())
		{
			m_pImpl->pDepthStencilState = state->second.Get();
		}
		else
		{
			AUTOPROFILE_DESC(GraphicsPipelineState_DepthStencilState_Create, Math::ToHex(stateHash));
			
			ComPtr<ID3D11DepthStencilState>& pState = pImpl->m_DepthStencilStates[stateHash];
			HR(pImpl->GetDevice()->CreateDepthStencilState(&m_pImpl->DepthStencilStateDesc, pState.GetAddressOf()));
			m_pImpl->pDepthStencilState = pState.Get();
		}
	}
	if(pImpl->m_BlendStateDirty || m_IsCreated == false)
	{
		//Blend State
		hasUpdated = true;
		pImpl->m_BlendStateDirty = false;

		//Check if the blend state already exists in the cache
		int32 stateHash =
			(unsigned char)m_pImpl->BlendMode << 0
			| (unsigned char)m_pImpl->BlendStateDesc.AlphaToCoverageEnable << 8
			| (unsigned char)m_pImpl->ColorWriteMask << 16;

		auto state = pImpl->m_BlendStates.find(stateHash);
		if (state != pImpl->m_BlendStates.end())
		{
			m_pImpl->pBlendState = state->second.Get();
		}
		else
		{
			AUTOPROFILE_DESC(GraphicsPipelineState_BlendState_Create, Math::ToHex(stateHash));

			ComPtr<ID3D11BlendState>& pState = pImpl->m_BlendStates[stateHash];
			HR(pImpl->GetDevice()->CreateBlendState(&m_pImpl->BlendStateDesc, pState.GetAddressOf()));
			m_pImpl->pBlendState = pState.Get();
		}
	}
	if(pImpl->m_RasterizerStateDirty || m_IsCreated == false)
	{
		//Rasterizer State
		hasUpdated = true;
		pImpl->m_RasterizerStateDirty = false;

		int32 stateHash =
			(unsigned char)m_pImpl->RasterizerStateDesc.ScissorEnable<< 0
			| (unsigned char)m_pImpl->RasterizerStateDesc.MultisampleEnable << 1
			| (unsigned char)m_pImpl->RasterizerStateDesc.FillMode << 2
			| (unsigned char)m_pImpl->RasterizerStateDesc.CullMode << 6
			| (unsigned char)m_pImpl->RasterizerStateDesc.AntialiasedLineEnable << 10;

		auto state = pImpl->m_RasterizerStates.find(stateHash);
		if (state != pImpl->m_RasterizerStates.end())
		{
			m_pImpl->pRasterizerState = state->second.Get();
		}
		else
		{
			AUTOPROFILE_DESC(GraphicsPipelineState_RasterizerState_Create, Math::ToHex(stateHash));

			ComPtr<ID3D11RasterizerState>& pState = pImpl->m_RasterizerStates[stateHash];
			HR(pImpl->GetDevice()->CreateRasterizerState(&m_pImpl->RasterizerStateDesc, pState.GetAddressOf()));
			m_pImpl->pRasterizerState = pState.Get();
		}
	}

	if (pImpl->m_InputLayoutDirty || m_IsCreated == false)
	{
		//Rasterizer State
		hasUpdated = true;
		pImpl->m_InputLayoutDirty = false;

		//Calculate the input element description hash to find the correct input layout
		uint64 hash = 0;

		for (int i = 0; i < count; ++i)
		{
			if (pVertexBuffers[i])
			{
				hash <<= pVertexBuffers[i]->GetElements().size() * 10;
				hash |= pVertexBuffers[i]->GetBufferHash();
			}
			else
			{
				hash <<= 1;
			}
		}

		if (hash == 0)
		{
			pImpl->m_pDeviceContext->IASetInputLayout(nullptr);
		}
		else
		{
			auto pIt = pImpl->m_InputLayoutMap.find(hash);
			if (pIt != pImpl->m_InputLayoutMap.end())
			{
				m_pImpl->pInputLayout = pIt->second.Get();
			}
			else
			{
				AUTOPROFILE(GraphicsPipelineState_InputLayout_Create);

				checkf(m_pVertexShader, "[GraphicsCommandContext] No vertex shader set");

				std::vector<D3D11_INPUT_ELEMENT_DESC> elementDesc;

				for (int i = 0; i < count; ++i)
				{
					if (pVertexBuffers[i] == nullptr)
					{
						continue;
					}

					for (const VertexElement& e : pVertexBuffers[i]->GetElements())
					{
						D3D11_INPUT_ELEMENT_DESC desc;
						desc.SemanticName = D3DCommon::GetSemanticOfType(e.Semantic);
						desc.Format = D3DCommon::GetFormatOfType(e.Type);
						desc.AlignedByteOffset = e.Offset;
						desc.InputSlotClass = e.PerInstance ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
						desc.InputSlot = i;
						desc.InstanceDataStepRate = e.PerInstance ? 1 : 0;
						desc.SemanticIndex = e.Index;

						elementDesc.push_back(desc);
					}
				}
				const std::vector<char>& byteCode = m_pVertexShader->GetByteCode();

				ComPtr<ID3D11InputLayout>& pInputLayout = pImpl->m_InputLayoutMap[hash];
				HR(m_pGraphics->GetImpl()->GetDevice()->CreateInputLayout(elementDesc.data(), (UINT)elementDesc.size(), byteCode.data(), (UINT)byteCode.size(), pInputLayout.GetAddressOf()));
				m_pImpl->pInputLayout = pInputLayout.Get();
			}
		}
	}

	m_IsCreated = true;
}


void GraphicsPipelineState::ApplyShader(ShaderType type, ShaderVariation* pShader)
{
	AUTOPROFILE_DESC(GraphicsPipelineState_ApplyShader, pShader ? pShader->GetName() : "None");

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();

	bool buffersChanged = false;
	if (pShader != nullptr)
	{
		for (ConstantBuffer* pBuffer : pShader->GetConstantBuffers())
		{
			if (pBuffer)
			{
				pBuffer->Apply();
			}
		}

		AUTOPROFILE_DESC(GraphicsPipelineState_SetConstantBuffers, pShader->GetName());
		const auto& buffers = pShader->GetConstantBuffers();
		for (unsigned int i = 0; i < buffers.size(); ++i)
		{
			if (buffers[i] != m_CurrentConstBuffers[(unsigned int)type][i])
			{
				m_CurrentConstBuffers[(unsigned int)type][i] = buffers[i] ? buffers[i]->GetResource() : nullptr;
				buffersChanged = true;
			}
		}
	}

	switch (type)
	{
	case ShaderType::VertexShader:
		pImpl->m_pDeviceContext->VSSetShader(pShader ? (ID3D11VertexShader*)pShader->GetResource() : nullptr, nullptr, 0);
		if (pShader && buffersChanged)
		{
			pImpl->m_pDeviceContext->VSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
		}
		break;
	case ShaderType::PixelShader:
		pImpl->m_pDeviceContext->PSSetShader(pShader ? (ID3D11PixelShader*)pShader->GetResource() : nullptr, nullptr, 0);
		if (pShader && buffersChanged)
		{
			pImpl->m_pDeviceContext->PSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
		}
		break;
	case ShaderType::GeometryShader:
		pImpl->m_pDeviceContext->GSSetShader(pShader ? (ID3D11GeometryShader*)pShader->GetResource() : nullptr, nullptr, 0);
		if (pShader && buffersChanged)
		{
			pImpl->m_pDeviceContext->GSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
		}
		break;
	case ShaderType::DomainShader:
		pImpl->m_pDeviceContext->DSSetShader(pShader ? (ID3D11DomainShader*)pShader->GetResource() : nullptr, nullptr, 0);
		if (pShader && buffersChanged)
		{
			pImpl->m_pDeviceContext->DSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
		}
		break;
	case ShaderType::HullShader:
		pImpl->m_pDeviceContext->HSSetShader(pShader ? (ID3D11HullShader*)pShader->GetResource() : nullptr, nullptr, 0);
		if (pShader && buffersChanged)
		{
			pImpl->m_pDeviceContext->HSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
		}
		break;
	default:
		FLUX_LOG(Error, "[Graphics::SetShader] > Shader type not implemented");
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

	if (pipelineStateUpdated)
	{
		GraphicsImpl* pImpl = m_pGraphics->GetImpl();
		AUTOPROFILE(Graphics_PrepareDraw_UpdatePipelineState);
		pImpl->m_pDeviceContext->OMSetDepthStencilState(m_pImpl->pDepthStencilState, GetStencilRef());
		pImpl->m_pDeviceContext->RSSetState((m_pImpl->pRasterizerState));
		pImpl->m_pDeviceContext->OMSetBlendState(m_pImpl->pBlendState, nullptr, UINT_MAX);
		pImpl->m_pDeviceContext->IASetInputLayout(m_pImpl->pInputLayout);
	}
}

void GraphicsPipelineState::SetBlendMode(const BlendMode& blendMode, const bool alphaToCoverage)
{
	if (blendMode != m_pImpl->BlendMode || alphaToCoverage != (bool)m_pImpl->BlendStateDesc.AlphaToCoverageEnable)
	{
		m_pImpl->BlendMode = blendMode;
		m_pImpl->BlendStateDesc.RenderTarget[0] = D3D11RenderTargetBlendDesc(blendMode, (unsigned char)m_pImpl->ColorWriteMask);
		m_pImpl->BlendStateDesc.AlphaToCoverageEnable = alphaToCoverage;

		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_BlendStateDirty = true;
	}
}

void GraphicsPipelineState::SetColorWrite(const ColorWrite colorWriteMask /*= ColorWrite::ALL*/)
{
	if (m_pImpl->ColorWriteMask != colorWriteMask)
	{
		m_pImpl->ColorWriteMask = colorWriteMask;
		m_pImpl->BlendStateDesc.RenderTarget[0] = D3D11RenderTargetBlendDesc(m_pImpl->BlendMode, (unsigned char)colorWriteMask);
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_BlendStateDirty = true;
	}
}

void GraphicsPipelineState::SetDepthEnabled(const bool enabled)
{
	if (enabled != (bool)m_pImpl->DepthStencilStateDesc.DepthEnable)
	{
		m_pImpl->DepthStencilStateDesc.DepthEnable = enabled;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
}

void GraphicsPipelineState::SetDepthWrite(const bool enabled)
{
	D3D11_DEPTH_WRITE_MASK mask = enabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	if (mask != m_pImpl->DepthStencilStateDesc.DepthWriteMask)
	{
		m_pImpl->DepthStencilStateDesc.DepthWriteMask = mask;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
}

void GraphicsPipelineState::SetDepthTest(const CompareMode& comparison)
{
	D3D11_COMPARISON_FUNC func = D3D11ComparisonFunction(comparison);
	if (func != m_pImpl->DepthStencilStateDesc.DepthFunc)
	{
		m_pImpl->DepthStencilStateDesc.DepthFunc = func;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
}

void GraphicsPipelineState::SetStencilTest(bool stencilEnabled, const CompareMode mode, const StencilOperation pass, const StencilOperation fail, const StencilOperation zFail, const unsigned int stencilRef, const unsigned char compareMask, unsigned char writeMask)
{
	m_StencilRef = stencilRef;

	if (stencilEnabled != (bool)m_pImpl->DepthStencilStateDesc.StencilEnable)
	{
		m_pImpl->DepthStencilStateDesc.StencilEnable = stencilEnabled;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}

	D3D11_COMPARISON_FUNC testMode = D3D11ComparisonFunction(mode);
	if (testMode != m_pImpl->DepthStencilStateDesc.FrontFace.StencilFunc)
	{
		m_pImpl->DepthStencilStateDesc.FrontFace.StencilFunc = testMode;
		m_pImpl->DepthStencilStateDesc.BackFace.StencilFunc = testMode;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}

	D3D11_STENCIL_OP passOp = D3D11StencilOperation(pass);
	if (passOp != m_pImpl->DepthStencilStateDesc.FrontFace.StencilPassOp)
	{
		m_pImpl->DepthStencilStateDesc.FrontFace.StencilPassOp = passOp;
		m_pImpl->DepthStencilStateDesc.BackFace.StencilPassOp = passOp;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}

	D3D11_STENCIL_OP failOp = D3D11StencilOperation(fail);
	if (failOp != m_pImpl->DepthStencilStateDesc.FrontFace.StencilFailOp)
	{
		m_pImpl->DepthStencilStateDesc.FrontFace.StencilFailOp = passOp;
		m_pImpl->DepthStencilStateDesc.BackFace.StencilFailOp = passOp;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}

	D3D11_STENCIL_OP zFailOp = D3D11StencilOperation(zFail);
	if (zFailOp != m_pImpl->DepthStencilStateDesc.FrontFace.StencilDepthFailOp)
	{
		m_pImpl->DepthStencilStateDesc.FrontFace.StencilDepthFailOp = zFailOp;
		m_pImpl->DepthStencilStateDesc.BackFace.StencilDepthFailOp = zFailOp;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
	if (compareMask != m_pImpl->DepthStencilStateDesc.StencilReadMask)
	{
		m_pImpl->DepthStencilStateDesc.StencilReadMask = compareMask;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
	if (writeMask != m_pImpl->DepthStencilStateDesc.StencilWriteMask)
	{
		m_pImpl->DepthStencilStateDesc.StencilWriteMask = writeMask;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
}

void GraphicsPipelineState::SetFillMode(FillMode fillMode)
{
	D3D11_FILL_MODE f = D3D11FillMode(fillMode);
	if (f != m_pImpl->RasterizerStateDesc.FillMode)
	{
		m_pImpl->RasterizerStateDesc.FillMode = f;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_RasterizerStateDirty = true;
	}
}

void GraphicsPipelineState::SetCullMode(CullMode cullMode)
{
	D3D11_CULL_MODE c = D3D11CullMode(cullMode);
	if (c != m_pImpl->RasterizerStateDesc.CullMode)
	{
		m_pImpl->RasterizerStateDesc.CullMode = c;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_RasterizerStateDirty = true;
	}
}

void GraphicsPipelineState::SetLineAntialias(bool lineAntiAlias)
{
	if (lineAntiAlias != (bool)m_pImpl->RasterizerStateDesc.AntialiasedLineEnable)
	{
		m_pImpl->RasterizerStateDesc.AntialiasedLineEnable = lineAntiAlias;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_RasterizerStateDirty = true;
	}
}

void GraphicsPipelineState::SetScissorEnabled(bool enabled)
{
	if (enabled != (bool)m_pImpl->RasterizerStateDesc.ScissorEnable)
	{
		m_pImpl->RasterizerStateDesc.ScissorEnable = enabled;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_RasterizerStateDirty = true;
	}
}

void GraphicsPipelineState::SetMultisampleEnabled(bool enabled)
{
	if (enabled != (bool)m_pImpl->RasterizerStateDesc.MultisampleEnable)
	{
		m_pImpl->RasterizerStateDesc.MultisampleEnable = enabled;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_RasterizerStateDirty = true;
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

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();

	bool buffersChanged = false;
	if (pShader != nullptr)
	{
		for (ConstantBuffer* pBuffer : pShader->GetConstantBuffers())
		{
			if (pBuffer)
			{
				pBuffer->Apply();
			}
		}

		AUTOPROFILE_DESC(GraphicsPipelineState_SetConstantBuffers, pShader->GetName());
		const auto& buffers = pShader->GetConstantBuffers();
		for (unsigned int i = 0; i < buffers.size(); ++i)
		{
			if (buffers[i] != m_CurrentConstBuffers[(unsigned int)type][i])
			{
				m_CurrentConstBuffers[(unsigned int)type][i] = buffers[i] ? buffers[i]->GetResource() : nullptr;
				buffersChanged = true;
			}
		}
	}

	switch (type)
	{
	case ShaderType::ComputeShader:
		pImpl->m_pDeviceContext->CSSetShader(pShader ? (ID3D11ComputeShader*)pShader->GetResource() : nullptr, nullptr, 0);
		if (pShader && buffersChanged)
		{
			pImpl->m_pDeviceContext->CSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
		}
		break;
	default:
		FLUX_LOG(Error, "[Graphics::SetShader] > Shader type not implemented");
		return;
	}
}