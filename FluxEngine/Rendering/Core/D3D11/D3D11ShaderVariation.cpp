#include "FluxEngine.h"
#include "D3D11GraphicsImpl.h"
#include "../ShaderVariation.h"
#include "../Graphics.h"
#include "../Shader.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

bool ShaderVariation::Create()
{
	AUTOPROFILE_DESC(ShaderVariation_Create, m_pParentShader->GetName());
	m_Name = m_pParentShader->GetShaderName() + "_" + Shader::GetEntryPoint(m_ShaderType);
	Graphics* pGraphics = GetSubsystem<Graphics>();
	if (!Compile(pGraphics))
	{
		FLUX_LOG(Warning, "[ShaderVariation::Create()] > Failed to compile shader");
		return false;
	}

	if (m_ShaderByteCode.size() == 0)
	{
		FLUX_LOG(Warning, "[ShaderVariation::Create()] > Shader byte code is empty");
		return false;
	}

	SafeRelease(m_pShaderObject);

	if (!CreateShader(pGraphics, m_ShaderType))
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
		return false;

	// Set the entrypoint, profile and flags according to the shader being compiled
	std::string entry = Shader::GetEntryPoint(m_ShaderType);
	const char* profile = nullptr;
	unsigned flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

#ifdef _DEBUG
	flags |= D3DCOMPILE_DEBUG;
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	std::vector<std::string> defines = m_Defines;
	defines.push_back("D3D11");

	switch (m_ShaderType)
	{
	case ShaderType::VertexShader:
		defines.push_back("COMPILE_VS");
		profile = "vs_4_0";
		break;
	case ShaderType::PixelShader:
		defines.push_back("COMPILE_PS");
		profile = "ps_4_0";
		flags |= D3DCOMPILE_PREFER_FLOW_CONTROL;
		break;
	case ShaderType::GeometryShader:
		defines.push_back("COMPILE_GS");
		profile = "gs_4_0";
		break;
	case ShaderType::ComputeShader:
		defines.push_back("COMPILE_CS");
		profile = "cs_4_0";
	default:
		break;
	}

	std::vector<D3D_SHADER_MACRO> macros;
	for (const std::string& define : defines)
	{
		D3D_SHADER_MACRO macro;
		//Check if the define has a value
		size_t assignmentOp = define.find('=');
		if (assignmentOp != std::string::npos)
		{
			std::string name = define.substr(0, assignmentOp);
			std::string definition = define.substr(assignmentOp + 1);
			macro.Name = name.c_str();
			macro.Definition = definition.c_str();
		}
		else
		{
			macro.Name = define.c_str();
			macro.Definition = "1";
		}
		macros.push_back(macro);
	}

	D3D_SHADER_MACRO endMacro;
	endMacro.Name = 0;
	endMacro.Definition = 0;
	macros.push_back(endMacro);

	ComPtr<ID3DBlob> pShaderCode, pErrorBlob;

	HRESULT hr = D3DCompile(source.c_str(), source.size(), m_Name.c_str(), macros.data(), 0, entry.c_str(), profile, flags, 0, pShaderCode.GetAddressOf(), pErrorBlob.GetAddressOf());
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
	HR(D3DStripShader(m_ShaderByteCode.data(), m_ShaderByteCode.size(), D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS, pStrippedCode.GetAddressOf()))
		m_ShaderByteCode.resize((unsigned)pStrippedCode->GetBufferSize());
	memcpy(&m_ShaderByteCode[0], pStrippedCode->GetBufferPointer(), m_ShaderByteCode.size());
#endif // !_DEBUG

	return true;
}

void ShaderVariation::ShaderReflection(char* pBuffer, unsigned bufferSize, Graphics* pGraphics)
{
	AUTOPROFILE(ShaderVariation_ShaderReflection);

	ComPtr<ID3D11ShaderReflection> pShaderReflection;
	D3D11_SHADER_DESC shaderDesc;

	HR(D3DReflect(pBuffer, bufferSize, IID_ID3D11ShaderReflection, (void**)pShaderReflection.GetAddressOf()));
	pShaderReflection->GetDesc(&shaderDesc);

	std::map<std::string, UINT> cbRegisterMap;

	for (unsigned i = 0; i < shaderDesc.BoundResources; ++i)
	{
		D3D11_SHADER_INPUT_BIND_DESC resourceDesc;
		pShaderReflection->GetResourceBindingDesc(i, &resourceDesc);
		std::string resourceName(resourceDesc.Name);
		if (resourceDesc.Type == D3D_SIT_CBUFFER)
			cbRegisterMap[resourceName] = resourceDesc.BindPoint;
	}

	for (unsigned int c = 0; c < shaderDesc.ConstantBuffers; ++c)
	{
		ID3D11ShaderReflectionConstantBuffer* pReflectionConstantBuffer = pShaderReflection->GetConstantBufferByIndex(c);
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		pReflectionConstantBuffer->GetDesc(&bufferDesc);
		unsigned cbRegister = cbRegisterMap[std::string(bufferDesc.Name)];

		if (cbRegister >= m_ConstantBuffers.size())
		{
			FLUX_LOG(Error, "[ShaderVariation::ShaderReflection] > The buffer '%s' with register index '%i' exceeds the maximum amount (%i) of constant buffers. See 'ShaderParameterType::MAX'",
				bufferDesc.Name,
				cbRegister,
				ShaderParameterType::MAX);
			return;
		}

		ConstantBuffer* pConstantBuffer = pGraphics->GetOrCreateConstantBuffer(m_ShaderType, cbRegister, bufferDesc.Size);
		m_ConstantBuffers[cbRegister] = pConstantBuffer;
		m_ConstantBufferSizes[cbRegister] = bufferDesc.Size;

		for (unsigned v = 0; v < bufferDesc.Variables; ++v)
		{
			ID3D11ShaderReflectionVariable* pVariable = pReflectionConstantBuffer->GetVariableByIndex(v);
			D3D11_SHADER_VARIABLE_DESC variableDesc;
			pVariable->GetDesc(&variableDesc);

			ShaderParameter parameter = {};
			parameter.Name = variableDesc.Name;
			parameter.Offset = variableDesc.StartOffset;
			parameter.Type = m_ShaderType;
			parameter.Size = variableDesc.Size;
			parameter.Buffer = cbRegister;
			parameter.pBuffer = pConstantBuffer;
			m_ShaderParameters[parameter.Name] = parameter;
		}
	}
}

bool ShaderVariation::CreateShader(Graphics* pGraphics, const ShaderType type)
{
	AUTOPROFILE_DESC(ShaderVariation_CreateShader, m_Name);

	if (m_ShaderByteCode.size() == 0)
	{
		return false;
	}
	switch (type)
	{
	case ShaderType::VertexShader:
		HR(pGraphics->GetImpl()->GetDevice()->CreateVertexShader(m_ShaderByteCode.data(), m_ShaderByteCode.size(), nullptr, (ID3D11VertexShader**)&m_pShaderObject))
		break;
	case ShaderType::PixelShader:
		HR(pGraphics->GetImpl()->GetDevice()->CreatePixelShader(m_ShaderByteCode.data(), m_ShaderByteCode.size(), nullptr, (ID3D11PixelShader**)&m_pShaderObject))
		break;
	case ShaderType::GeometryShader:
		HR(pGraphics->GetImpl()->GetDevice()->CreateGeometryShader(m_ShaderByteCode.data(), m_ShaderByteCode.size(), nullptr, (ID3D11GeometryShader**)&m_pShaderObject))
		break;
	case ShaderType::ComputeShader:
		HR(pGraphics->GetImpl()->GetDevice()->CreateComputeShader(m_ShaderByteCode.data(), m_ShaderByteCode.size(), nullptr, (ID3D11ComputeShader**)&m_pShaderObject))
		break;
	}
	return true;
}
