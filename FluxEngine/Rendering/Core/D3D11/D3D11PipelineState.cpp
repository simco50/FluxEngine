#include "FluxEngine.h"
#include "../PipelineState.h"
#include "../Graphics.h"
#include "../ShaderVariation.h"
#include "../ConstantBuffer.h"
#include "../VertexBuffer.h"
#include "../D3DCommon/D3DDefines.h"
#include "D3D11GraphicsImpl.h"
#include "D3D11Helpers.h"

bool PipelineState::SetParameter(StringHash hash, const void* pData)
{
	LoadShaderParameters();
	auto pParameter = m_ShaderParameters.find(hash);
	if (pParameter == m_ShaderParameters.end())
	{
		return false;
	}
	return pParameter->second->pBuffer->SetData(pData, pParameter->second->Offset, pParameter->second->Size, false);
}

bool PipelineState::SetParameter(StringHash hash, const void* pData, int size)
{
	LoadShaderParameters();
	auto pParameter = m_ShaderParameters.find(hash);
	if (pParameter == m_ShaderParameters.end())
	{
		return false;
	}
	return pParameter->second->pBuffer->SetData(pData, pParameter->second->Offset, size, false);
}

void GraphicsPipelineState::Finalize(bool& hasUpdated)
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
			(unsigned char)m_DepthEnabled << 0
			| (unsigned char)m_DepthWrite << 1
			| (unsigned char)m_DepthCompareMode << 2
			| (unsigned char)m_StencilTestEnabled << 6
			| (unsigned char)m_StencilTestMode << 7
			| (unsigned char)m_StencilTestPassOperation << 11
			| (unsigned char)m_StencilTestFailOperation << 15
			| (unsigned char)m_StencilTestZFailOperation << 19
			| (unsigned char)m_StencilCompareMask << 23
			| (unsigned char)m_StencilWriteMask << 27;

		auto state = pImpl->m_DepthStencilStates.find(stateHash);
		if (state != pImpl->m_DepthStencilStates.end())
		{
			m_Data.pDepthStencilState = state->second.Get();
		}
		else
		{
			AUTOPROFILE_DESC(PipelineState_DepthStencilState_Create, Math::ToHex(stateHash));

			D3D11_DEPTH_STENCIL_DESC desc = {};
			desc.DepthEnable = m_DepthEnabled;
			desc.DepthWriteMask = m_DepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
			desc.DepthFunc = D3D11ComparisonFunction(m_DepthCompareMode);
			desc.StencilEnable = m_StencilTestEnabled;
			desc.StencilReadMask = m_StencilCompareMask;
			desc.StencilWriteMask = m_StencilWriteMask;
			desc.FrontFace.StencilFunc = D3D11ComparisonFunction(m_StencilTestMode);
			desc.BackFace.StencilFunc = desc.FrontFace.StencilFunc;
			desc.FrontFace.StencilPassOp = D3D11StencilOperation(m_StencilTestPassOperation);
			desc.BackFace.StencilPassOp = desc.FrontFace.StencilPassOp;
			desc.FrontFace.StencilFailOp = D3D11StencilOperation(m_StencilTestFailOperation);
			desc.BackFace.StencilFailOp = desc.FrontFace.StencilFailOp;
			desc.FrontFace.StencilDepthFailOp = D3D11StencilOperation(m_StencilTestZFailOperation);
			desc.BackFace.StencilDepthFailOp = desc.FrontFace.StencilDepthFailOp;

			ComPtr<ID3D11DepthStencilState>& pState = pImpl->m_DepthStencilStates[stateHash];
			HR(pImpl->GetDevice()->CreateDepthStencilState(&desc, pState.GetAddressOf()));
			m_Data.pDepthStencilState = pState.Get();
		}
	}
	if(pImpl->m_BlendStateDirty || m_IsCreated == false)
	{
		//Blend State
		hasUpdated = true;
		pImpl->m_BlendStateDirty = false;

		//Check if the blend state already exists in the cache
		unsigned int stateHash =
			(unsigned char)m_BlendMode << 0
			| (unsigned char)m_AlphaToCoverage << 8
			| (unsigned char)m_ColorWriteMask << 16;

		auto state = pImpl->m_BlendStates.find(stateHash);
		if (state != pImpl->m_BlendStates.end())
		{
			m_Data.pBlendState = state->second.Get();
		}
		else
		{
			AUTOPROFILE_DESC(PipelineState_BlendState_Create, Math::ToHex(stateHash));

			D3D11_BLEND_DESC desc = {};
			desc.AlphaToCoverageEnable = m_AlphaToCoverage;
			//IndependentBlendEnable == false as we only use blending on one rendertarget
			desc.IndependentBlendEnable = false;
			desc.RenderTarget[0] = D3D11RenderTargetBlendDesc(m_BlendMode, (unsigned char)m_ColorWriteMask);

			ComPtr<ID3D11BlendState>& pState = pImpl->m_BlendStates[stateHash];
			HR(pImpl->GetDevice()->CreateBlendState(&desc, pState.GetAddressOf()));
			m_Data.pBlendState = pState.Get();
		}
	}
	if(pImpl->m_RasterizerStateDirty || m_IsCreated == false)
	{
		//Rasterizer State
		hasUpdated = true;
		pImpl->m_RasterizerStateDirty = false;

		unsigned int stateHash =
			(unsigned char)m_ScissorEnabled << 0
			| (unsigned char)m_MultisampleEnabled << 1
			| (unsigned char)m_FillMode << 2
			| (unsigned char)m_CullMode << 6
			| (unsigned char)m_LineAntiAlias << 10;

		auto state = pImpl->m_RasterizerStates.find(stateHash);
		if (state != pImpl->m_RasterizerStates.end())
		{
			m_Data.pRasterizerState = state->second.Get();
		}
		else
		{
			AUTOPROFILE_DESC(PipelineState_RasterizerState_Create, Math::ToHex(stateHash));

			D3D11_RASTERIZER_DESC desc;
			ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));
			desc.AntialiasedLineEnable = m_LineAntiAlias;
			desc.CullMode = D3D11CullMode(m_CullMode);
			desc.DepthBias = 0;
			desc.DepthBiasClamp = 0.0f;
			desc.DepthClipEnable = true;
			desc.FillMode = D3D11FillMode(m_FillMode);
			desc.FrontCounterClockwise = false;
			desc.MultisampleEnable = m_MultisampleEnabled;
			desc.ScissorEnable = m_ScissorEnabled;
			desc.SlopeScaledDepthBias = 0.0f;

			ComPtr<ID3D11RasterizerState>& pState = pImpl->m_RasterizerStates[stateHash];
			HR(pImpl->GetDevice()->CreateRasterizerState(&desc, pState.GetAddressOf()));
			m_Data.pRasterizerState = pState.Get();
		}
	}

	LoadShaderParameters();

	m_IsCreated = true;
}


void PipelineState::ApplyShader(ShaderType type, ShaderVariation* pShader)
{
	AUTOPROFILE_DESC(PipelineState_UpdateShader, pShader ? pShader->GetName() : "None");

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();

	if (pShader != nullptr)
	{
		for (ConstantBuffer* pBuffer : pShader->GetConstantBuffers())
		{
			if (pBuffer)
			{
				pBuffer->Apply();
			}
		}
	}

	switch (type)
	{
	case ShaderType::VertexShader:
		pImpl->m_pDeviceContext->VSSetShader(pShader ? (ID3D11VertexShader*)pShader->GetResource() : nullptr, nullptr, 0);
		break;
	case ShaderType::PixelShader:
		pImpl->m_pDeviceContext->PSSetShader(pShader ? (ID3D11PixelShader*)pShader->GetResource() : nullptr, nullptr, 0);
		break;
	case ShaderType::GeometryShader:
		pImpl->m_pDeviceContext->GSSetShader(pShader ? (ID3D11GeometryShader*)pShader->GetResource() : nullptr, nullptr, 0);
		break;
	case ShaderType::ComputeShader:
		pImpl->m_pDeviceContext->CSSetShader(pShader ? (ID3D11ComputeShader*)pShader->GetResource() : nullptr, nullptr, 0);
		break;
	case ShaderType::DomainShader:
		pImpl->m_pDeviceContext->DSSetShader(pShader ? (ID3D11DomainShader*)pShader->GetResource() : nullptr, nullptr, 0);
		break;
	case ShaderType::HullShader:
		pImpl->m_pDeviceContext->HSSetShader(pShader ? (ID3D11HullShader*)pShader->GetResource() : nullptr, nullptr, 0);
		break;
	default:
		FLUX_LOG(Error, "[Graphics::SetShader] > Shader type not implemented");
		return;
	}

	if (pShader)
	{
		AUTOPROFILE_DESC(PipelineState_SetConstantBuffers, pShader->GetName());
		bool buffersChanged = false;
		const auto& buffers = pShader->GetConstantBuffers();
		for (unsigned int i = 0; i < buffers.size(); ++i)
		{
			if (buffers[i] != m_CurrentConstBuffers[(unsigned int)type][i])
			{
				m_CurrentConstBuffers[(unsigned int)type][i] = buffers[i] ? buffers[i]->GetResource() : nullptr;
				buffersChanged = true;
			}
		}
		if (buffersChanged)
		{
			switch (type)
			{
			case ShaderType::VertexShader:
				pImpl->m_pDeviceContext->VSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
				break;
			case ShaderType::PixelShader:
				pImpl->m_pDeviceContext->PSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
				break;
			case ShaderType::GeometryShader:
				pImpl->m_pDeviceContext->GSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
				break;
			case ShaderType::ComputeShader:
				pImpl->m_pDeviceContext->CSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
				break;
			case ShaderType::DomainShader:
				pImpl->m_pDeviceContext->DSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
				break;
			case ShaderType::HullShader:
				pImpl->m_pDeviceContext->HSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
				break;
			default:
				break;
			}
		}
	}
}

void GraphicsPipelineState::ApplyInputLayout(VertexBuffer** pVertexBuffers, int count)
{
	AUTOPROFILE(GraphicsPipelineState_ApplyInputLayout);
	//Calculate the input element description hash to find the correct input layout
	unsigned long long hash = 0;

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

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();

	if (hash == 0)
	{
		pImpl->m_pDeviceContext->IASetInputLayout(nullptr);
	}
	else
	{
		auto pIt = pImpl->m_InputLayoutMap.find(hash);
		if (pIt != pImpl->m_InputLayoutMap.end())
		{
			pImpl->m_pDeviceContext->IASetInputLayout(pIt->second.Get());
		}
		else
		{
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
			pImpl->m_pDeviceContext->IASetInputLayout(pInputLayout.Get());
		}
	}
}

void GraphicsPipelineState::Apply(VertexBuffer** pVertexBuffers, int count)
{
	AUTOPROFILE(GraphicsPipelineState_Apply);

	ApplyInputLayout(pVertexBuffers, count);

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
	Finalize(pipelineStateUpdated);

	if (pipelineStateUpdated)
	{
		GraphicsImpl* pImpl = m_pGraphics->GetImpl();
		AUTOPROFILE(Graphics_PrepareDraw_UpdatePipelineState);
		pImpl->m_pDeviceContext->OMSetDepthStencilState((ID3D11DepthStencilState*)m_Data.pDepthStencilState, GetStencilRef());
		pImpl->m_pDeviceContext->RSSetState((ID3D11RasterizerState*)m_Data.pRasterizerState);
		pImpl->m_pDeviceContext->OMSetBlendState((ID3D11BlendState*)m_Data.pBlendState, nullptr, UINT_MAX);
	}
}

//Compute Pipeline state

void ComputePipelineState::Finalize(bool& hasUpdated)
{
	hasUpdated = true;
	m_ShaderParameters.clear();
	LoadShaderParametersForShader(m_pComputeShader);
}