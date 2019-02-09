#include "FluxEngine.h"
#include "../ShaderVariation.h"
#include "../Graphics.h"
#include "../Shader.h"

#include <d3dcompiler.h>
#include "../D3DCommon/D3DHelpers.h"
#include "D3D12GraphicsImpl.h"

bool ShaderVariation::Create()
{
	AUTOPROFILE_DESC(ShaderVariation_Create, m_pParentShader->GetFilePath());
	m_Name = m_pParentShader->GetName() + "_" + Shader::GetEntryPoint(m_ShaderType);
	if (!Compile(m_pGraphics))
	{
		FLUX_LOG(Warning, "[ShaderVariation::Create()] > Failed to compile shader");
		return false;
	}

	if (m_ShaderByteCode.empty())
	{
		FLUX_LOG(Warning, "[ShaderVariation::Create()] > Shader byte code is empty");
		return false;
	}

	SafeRelease(m_pResource);

	if (!CreateShader(m_pGraphics, m_ShaderType))
	{
		return false;
	}

	return true;
}

bool ShaderVariation::Compile(Graphics* pGraphics)
{
	AUTOPROFILE_DESC(ShaderVariation_Compile, m_Name);

	const std::string& source = m_pParentShader->GetSource();
	if (source.length() == 0)
	{
		return false;
	}

	// Set the entrypoint, profile and flags according to the shader being compiled
	std::string entry = Shader::GetEntryPoint(m_ShaderType);
	unsigned flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

#ifdef _DEBUG
	flags |= D3DCOMPILE_DEBUG;
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	flags |= D3DCOMPILE_PREFER_FLOW_CONTROL;
#else
	flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	std::vector<std::string> defines = m_Defines;
	std::vector<std::string> defineValues;
	defines.emplace_back("D3D11");
	defines.push_back(Printf("LIGHT_COUNT=%d", GraphicsConstants::MAX_LIGHTS));

	m_ShaderModel = GetDesiredShaderModel();

	std::string profile;
	switch (m_ShaderType)
	{
	case ShaderType::VertexShader:
		defines.emplace_back("COMPILE_VS");
		profile = Printf("vs_%d_%d", m_ShaderModel.MajVersion, m_ShaderModel.MinVersion);
		break;
	case ShaderType::PixelShader:
		defines.emplace_back("COMPILE_PS");
		profile = Printf("ps_%d_%d", m_ShaderModel.MajVersion, m_ShaderModel.MinVersion);
		break;
	case ShaderType::GeometryShader:
		defines.emplace_back("COMPILE_GS");
		profile = Printf("gs_%d_%d", m_ShaderModel.MajVersion, m_ShaderModel.MinVersion);
		break;
	case ShaderType::ComputeShader:
		defines.emplace_back("COMPILE_CS");
		profile = Printf("cs_%d_%d", m_ShaderModel.MajVersion, m_ShaderModel.MinVersion);
		break;
	case ShaderType::DomainShader:
		defines.emplace_back("COMPILE_DS");
		profile = Printf("ds_%d_%d", m_ShaderModel.MajVersion, m_ShaderModel.MinVersion);
		break;
	case ShaderType::HullShader:
		defines.emplace_back("COMPILE_HS");
		profile = Printf("hs_%d_%d", m_ShaderModel.MajVersion, m_ShaderModel.MinVersion);
		break;
	default:
		break;
	}

	for (size_t i = 0; i < defines.size(); ++i)
	{
		//Check if the define has a value
		size_t assignmentOp = defines[i].find('=');
		if (assignmentOp != std::string::npos)
		{
			defineValues.emplace_back(defines[i].substr(assignmentOp + 1));
			defines[i] = defines[i].substr(0, assignmentOp);
		}
		else
		{
			defineValues.emplace_back("1");
		}
	}
	std::vector<D3D_SHADER_MACRO> macros;
	for (size_t i = 0; i < defines.size(); ++i)
	{
		macros.push_back({ defines[i].c_str(), defineValues[i].c_str() });
	}

	D3D_SHADER_MACRO endMacro;
	endMacro.Name = nullptr;
	endMacro.Definition = nullptr;
	macros.push_back(endMacro);

	ComPtr<ID3DBlob> pShaderCode, pErrorBlob;

	HRESULT hr = D3DCompile(source.c_str(), source.size(), m_Name.c_str(), macros.data(), nullptr, entry.c_str(), profile.c_str(), flags, 0, pShaderCode.GetAddressOf(), pErrorBlob.GetAddressOf());
	if (hr != S_OK)
	{
		std::string error = D3DBlobToString(pErrorBlob.Get());
		FLUX_LOG(Warning, error.c_str());
		return false;
	}
	D3DBlobToVector(pShaderCode.Get(), m_ShaderByteCode);
	ShaderReflection(m_ShaderByteCode.data(), (unsigned int)m_ShaderByteCode.size(), pGraphics);

#ifndef _DEBUG
	// Strip everything not necessary to use the shader
	ComPtr<ID3DBlob> pStrippedCode;
	HR(D3DStripShader(m_ShaderByteCode.data(), m_ShaderByteCode.size(), D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS, pStrippedCode.GetAddressOf()));
	m_ShaderByteCode.resize((unsigned)pStrippedCode->GetBufferSize());
	memcpy(&m_ShaderByteCode[0], pStrippedCode->GetBufferPointer(), m_ShaderByteCode.size());
#endif // !_DEBUG

	return true;
}

void ShaderVariation::ShaderReflection(char* pBuffer, unsigned bufferSize, Graphics* pGraphics)
{
	AUTOPROFILE(ShaderVariation_ShaderReflection);

	ComPtr<ID3D12ShaderReflection> pShaderReflection;
	D3D12_SHADER_DESC shaderDesc;

	HR(D3DReflect(pBuffer, bufferSize, IID_ID3D12ShaderReflection, (void**)pShaderReflection.GetAddressOf()));
	pShaderReflection->GetDesc(&shaderDesc);

	std::map<std::string, uint32> cbRegisterMap;

	for (unsigned i = 0; i < shaderDesc.BoundResources; ++i)
	{
		D3D12_SHADER_INPUT_BIND_DESC resourceDesc;
		pShaderReflection->GetResourceBindingDesc(i, &resourceDesc);

		switch (resourceDesc.Type)
		{
		case D3D_SIT_CBUFFER:
		case D3D_SIT_TBUFFER:
			cbRegisterMap[resourceDesc.Name] = resourceDesc.BindPoint;
			break;
		case D3D_SIT_TEXTURE:
		case D3D_SIT_SAMPLER:
		case D3D_SIT_UAV_RWTYPED:
		case D3D_SIT_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_BYTEADDRESS:
		case D3D_SIT_UAV_RWBYTEADDRESS:
		case D3D_SIT_UAV_APPEND_STRUCTURED:
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
		default:
			break;
		}
	}

	for (unsigned int c = 0; c < shaderDesc.ConstantBuffers; ++c)
	{
		ID3D12ShaderReflectionConstantBuffer* pReflectionConstantBuffer = pShaderReflection->GetConstantBufferByIndex(c);
		D3D12_SHADER_BUFFER_DESC bufferDesc;
		pReflectionConstantBuffer->GetDesc(&bufferDesc);
		uint32 cbRegister = cbRegisterMap[std::string(bufferDesc.Name)];
		checkf(cbRegister < m_ConstantBuffers.size(), "[ShaderVariation::ShaderReflection] > The buffer exceeds the maximum amount of constant buffers. See 'ShaderParameterType::MAX'");

		ConstantBuffer* pConstantBuffer = pGraphics->GetOrCreateConstantBuffer(cbRegister, bufferDesc.Size);
		m_ConstantBuffers[cbRegister] = pConstantBuffer;
		m_ConstantBufferSizes[cbRegister] = bufferDesc.Size;

		for (unsigned v = 0; v < bufferDesc.Variables; ++v)
		{
			ID3D12ShaderReflectionVariable* pVariable = pReflectionConstantBuffer->GetVariableByIndex(v);
			D3D12_SHADER_VARIABLE_DESC variableDesc;
			pVariable->GetDesc(&variableDesc);

			ShaderParameter parameter = {};
			parameter.Name = variableDesc.Name;
			parameter.Offset = variableDesc.StartOffset;
			parameter.Size = variableDesc.Size;
			parameter.Buffer = cbRegister;
			parameter.pBuffer = pConstantBuffer;
			m_ShaderParameters[StringHash(variableDesc.Name)] = parameter;
		}
	}
}

bool ShaderVariation::CreateShader(Graphics* /*pGraphics*/, const ShaderType /*type*/)
{
	AUTOPROFILE_DESC(ShaderVariation_CreateShader, m_Name);

	if (m_ShaderByteCode.size() == 0)
	{
		return false;
	}
	return true;
}

ShaderVariation::ShaderModel ShaderVariation::GetDesiredShaderModel()
{
	ShaderVariation::ShaderModel model;
	D3D12_FEATURE_DATA_SHADER_MODEL shaderModel;
	shaderModel.HighestShaderModel = D3D_SHADER_MODEL_5_1;
	HR(m_pGraphics->GetImpl()->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(D3D12_FEATURE_DATA_SHADER_MODEL)));
	switch (shaderModel.HighestShaderModel)
	{
	case D3D_SHADER_MODEL_5_1:
		model = ShaderModel(5, 1);
		break;
	case D3D_SHADER_MODEL_6_0:
		model = ShaderModel(6, 0);
		break;
	case D3D_SHADER_MODEL_6_1:
		model = ShaderModel(6, 1);
		break;
	case D3D_SHADER_MODEL_6_2:
		model = ShaderModel(6, 2);
		break;
	case D3D_SHADER_MODEL_6_3:
		model = ShaderModel(6, 3);
		break;
	case D3D_SHADER_MODEL_6_4:
		model = ShaderModel(6, 4);
		break;
	default:
		check(false);
	}

	return model;
}