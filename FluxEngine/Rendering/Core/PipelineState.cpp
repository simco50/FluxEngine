#include "FluxEngine.h"
#include "PipelineState.h"
#include "Graphics.h"
#include "D3D11/D3D11GraphicsImpl.h"
#include "ShaderVariation.h"
#include "ConstantBuffer.h"

/////////Pipeline State
////////////////////////////////////////////

PipelineState::PipelineState(Graphics* pGraphics)
	: m_pGraphics(pGraphics)
{

}

PipelineState::~PipelineState()
{

}

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

bool PipelineState::SetParameter(const std::string& name, const void* pData)
{
	return SetParameter(StringHash(name), pData);
}

void PipelineState::LoadShaderParametersForShader(ShaderVariation* pShader)
{
	if (pShader == nullptr)
	{
		return;
	}
	const std::map<StringHash, ShaderParameter>& parameters = pShader->GetParameters();
	for (const auto& parameter : parameters)
	{
		m_ShaderParameters[parameter.first] = &parameter.second;
	}
}

/////////Graphics Pipeline State
////////////////////////////////////////////

void GraphicsPipelineState::LoadShaderParameters()
{
	AUTOPROFILE(GraphicsPipelineState_LoadShaderParameters);
	if (m_ShaderParametersDirty)
	{
		m_ShaderParameters.clear();
		LoadShaderParametersForShader(m_pVertexShader);
		LoadShaderParametersForShader(m_pPixelShader);
		LoadShaderParametersForShader(m_pGeometryShader);
		LoadShaderParametersForShader(m_pHullShader);
		LoadShaderParametersForShader(m_pDomainShader);
		m_ShaderParametersDirty = false;
	}
}

void GraphicsPipelineState::SetVertexShader(ShaderVariation* pShader)
{
	m_pVertexShader = pShader;
	m_DirtyShaders.SetBit((int)ShaderType::VertexShader);
	m_ShaderParametersDirty = true;
}

void GraphicsPipelineState::SetPixelShader(ShaderVariation* pShader)
{
	m_pPixelShader = pShader;
	m_DirtyShaders.SetBit((int)ShaderType::PixelShader);
	m_ShaderParametersDirty = true;
}

void GraphicsPipelineState::SetGeometryShader(ShaderVariation* pShader)
{
	m_pGeometryShader = pShader;
	m_DirtyShaders.SetBit((int)ShaderType::GeometryShader);
	m_ShaderParametersDirty = true;
}

void GraphicsPipelineState::SetHullShader(ShaderVariation* pShader)
{
	m_pHullShader = pShader;
	m_DirtyShaders.SetBit((int)ShaderType::HullShader);
	m_ShaderParametersDirty = true;
}

void GraphicsPipelineState::SetDomainShader(ShaderVariation* pShader)
{
	m_pDomainShader = pShader;
	m_DirtyShaders.SetBit((int)ShaderType::DomainShader);
	m_ShaderParametersDirty = true;
}

void GraphicsPipelineState::ClearShaders()
{
	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
	m_pGeometryShader = nullptr;
	m_pHullShader = nullptr;
	m_pDomainShader = nullptr;
	m_DirtyShaders.SetAll();
	m_ShaderParametersDirty = true;
}

/////////Compute Pipeline State
////////////////////////////////////////////

ComputePipelineState::ComputePipelineState(Graphics* pGraphics)
	: PipelineState(pGraphics)
{

}

ComputePipelineState::~ComputePipelineState()
{

}

void ComputePipelineState::ClearShaders()
{
	m_pComputeShader = nullptr;
	m_DirtyShaders.SetAll();
	m_ShaderParametersDirty = true;
}

void ComputePipelineState::SetComputeShader(ShaderVariation* pShader)
{
	m_pComputeShader = pShader;
	m_DirtyShaders.SetBit((int)ShaderType::ComputeShader);
	m_ShaderParametersDirty = true;
}

void ComputePipelineState::LoadShaderParameters()
{
	AUTOPROFILE(ComputePipelineState_LoadShaderParameters);
	if (m_ShaderParametersDirty)
	{
		m_ShaderParameters.clear();
		LoadShaderParametersForShader(m_pComputeShader);
		m_ShaderParametersDirty = false;
	}
}
