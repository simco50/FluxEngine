#include "FluxEngine.h"
#include "../PipelineState.h"
#include "../Graphics.h"
#include "D3D11GraphicsImpl.h"
#include "D3D11Helpers.h"
#include "../ShaderVariation.h"
#include "../ConstantBuffer.h"

bool PipelineState::SetParameter(const std::string& name, const void* pData)
{
	return SetParameter(StringHash(name), pData);
}

bool PipelineState::SetParameter(StringHash hash, const void* pData)
{
	auto pParameter = m_ShaderParameters.find(hash);
	if (pParameter == m_ShaderParameters.end())
	{
		return false;
	}
	return pParameter->second->pBuffer->SetData(pData, pParameter->second->Offset, pParameter->second->Size, false);
}

void GraphicsPipelineState::Finalize(bool& hasUpdated)
{
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
			AUTOPROFILE_DESC(DepthStencilState_Create, Math::ToHex(stateHash));

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
			AUTOPROFILE_DESC(BlendState_Create, Math::ToHex(stateHash));

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
			AUTOPROFILE_DESC(RasterizerState_Create, Math::ToHex(stateHash));

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

	m_ShaderParameters.clear();
	LoadShaderParametersOfShader(m_pVertexShader);
	LoadShaderParametersOfShader(m_pPixelShader);
	LoadShaderParametersOfShader(m_pGeometryShader);
	LoadShaderParametersOfShader(m_pHullShader);
	LoadShaderParametersOfShader(m_pDomainShader);

	m_IsCreated = true;
}


void PipelineState::ApplyShader(ShaderType type)
{
	ShaderVariation* pShader = nullptr;

	AUTOPROFILE_DESC(Graphics_UpdateShader, pShader ? pShader->GetName() : "None");

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

#ifdef SHADER_GEOMETRY_ENABLE
	case ShaderType::GeometryShader:
		pImpl->m_pDeviceContext->GSSetShader(pShader ? (ID3D11GeometryShader*)pShader->GetResource() : nullptr, nullptr, 0);
		break;
#endif
#ifdef SHADER_COMPUTE_ENABLE
	case ShaderType::ComputeShader:
		pImpl->m_pDeviceContext->CSSetShader(pShader ? (ID3D11ComputeShader*)pShader->GetResource() : nullptr, nullptr, 0);
		break;
#endif
#ifdef SHADER_TESSELLATION_ENABLE
	case ShaderType::DomainShader:
		pImpl->m_pDeviceContext->DSSetShader(pShader ? (ID3D11DomainShader*)pShader->GetResource() : nullptr, nullptr, 0);
		break;
	case ShaderType::HullShader:
		pImpl->m_pDeviceContext->HSSetShader(pShader ? (ID3D11HullShader*)pShader->GetResource() : nullptr, nullptr, 0);
		break;
#endif
	default:
		FLUX_LOG(Error, "[Graphics::SetShader] > Shader type not implemented");
		return;
	}

	if (pShader)
	{
		AUTOPROFILE_DESC(Graphics_SetConstantBuffers, pShader->GetName());
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
#ifdef SHADER_GEOMETRY_ENABLE
			case ShaderType::GeometryShader:
				pImpl->m_pDeviceContext->GSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
				break;
#endif
#ifdef SHADER_COMPUTE_ENABLE
			case ShaderType::ComputeShader:
				pImpl->m_pDeviceContext->CSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
				break;
#endif
#ifdef SHADER_TESSELLATION_ENABLE
			case ShaderType::DomainShader:
				pImpl->m_pDeviceContext->DSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
				break;
			case ShaderType::HullShader:
				pImpl->m_pDeviceContext->HSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
				break;
#endif
			default:
				break;
			}
		}
	}
}

bool GraphicsPipelineState::SetVertexShader(ShaderVariation* pShader)
{
	m_pVertexShader = pShader;
	m_VertexShaderDirty = true;
	return true;
}

bool GraphicsPipelineState::SetPixelShader(ShaderVariation* pShader)
{
	m_pPixelShader = pShader;
	m_PixelShaderDirty = true;
	return true;
}

bool GraphicsPipelineState::SetGeometryShader(ShaderVariation* pShader)
{
	m_pGeometryShader = pShader;
	m_GeometryShaderDirty = true;
	return true;
}

bool GraphicsPipelineState::SetHullShader(ShaderVariation* pShader)
{
	m_pHullShader = pShader;
	m_HullShaderDirty = true;
	return true;
}

bool GraphicsPipelineState::SetDomainShader(ShaderVariation* pShader)
{
	m_pDomainShader = pShader;
	m_DomainShaderDirty = true;
	return true;
}

void ComputePipelineState::Finalize(bool& hasUpdated)
{
	hasUpdated = true;
	m_ShaderParameters.clear();
	LoadShaderParametersOfShader(m_pComputeShader);
}