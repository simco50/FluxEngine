#include "stdafx.h"
#include "ShaderVariation.h"
#include "Shader.h"
#include "Graphics.h"

ShaderVariation::ShaderVariation(Shader* pShader, const ShaderType type) :
	m_pParentShader(pShader),
	m_ShaderType(type)
{

}

ShaderVariation::~ShaderVariation()
{
	Release();
}

bool ShaderVariation::Create(Graphics* pGraphics)
{
	Compile();

	if (m_ShaderByteCode.size() == 0)
	{
		FLUX_LOG(ERROR, "[ShaderVariation::Create()] > Shader byte code is empty");
		return false;
	}

	switch (m_ShaderType)
	{
	case ShaderType::VertexShader:
		HR(pGraphics->GetDevice()->CreateVertexShader(m_ShaderByteCode.data(), m_ShaderByteCode.size(), nullptr, (ID3D11VertexShader**)&m_pShaderObject))
			break;
	case ShaderType::PixelShader:
		HR(pGraphics->GetDevice()->CreatePixelShader(m_ShaderByteCode.data(), m_ShaderByteCode.size(), nullptr, (ID3D11PixelShader**)&m_pShaderObject))
			break;
	default:
		break;
	}
	return true;
}

void ShaderVariation::Release()
{
	if (m_pShaderObject == nullptr)
		return;

	switch (m_ShaderType)
	{
	case ShaderType::VertexShader:
		((ID3D11VertexShader*)m_pShaderObject)->Release();
		break;
	case ShaderType::PixelShader:
		((ID3D11PixelShader*)m_pShaderObject)->Release();
		break;
	default:
		break;
	}
	m_pShaderObject = nullptr;
}

bool ShaderVariation::Compile()
{
	const string& source = m_pParentShader->GetSource(m_ShaderType);

	if (source.length() == 0)
		return false;

	// Set the entrypoint, profile and flags according to the shader being compiled
	const char* entryPoint = 0;
	const char* profile = 0;
	unsigned flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;

	vector<string> defines = m_Defines;
	defines.push_back("D3D11");

	switch (m_ShaderType)
	{
	case ShaderType::VertexShader:
		entryPoint = "VS";
		defines.push_back("COMPILE_VS");
		profile = "vs_4_0";
		break;
	case ShaderType::PixelShader:
		entryPoint = "PS";
		defines.push_back("COMPILE_PS");
		profile = "ps_4_0";
		flags |= D3DCOMPILE_PREFER_FLOW_CONTROL;
		break;
	default:
		break;
	}

	vector<D3D_SHADER_MACRO> macros;
	for (const string& define : defines)
	{
		D3D_SHADER_MACRO macro;
		macro.Definition = "1";
		macro.Name = define.c_str();
		macros.push_back(macro);
	}
	D3D_SHADER_MACRO endMacro;
	endMacro.Name = 0;
	endMacro.Definition = 0;
	macros.push_back(endMacro);

	ID3DBlob* shaderCode = 0;
	ID3DBlob* errorMsgs = 0;
	HRESULT hr = D3DCompile(source.c_str(), source.size(), "shader", macros.data(), 0, entryPoint, profile, flags, 0, &shaderCode, &errorMsgs);
	if (hr != S_OK)
	{
		string errorMessage((char*)errorMsgs->GetBufferPointer(), (char*)errorMsgs->GetBufferPointer() + errorMsgs->GetBufferSize());
		FLUX_LOG(ERROR, errorMessage);
		return false;
	}
	unsigned char* pBuffer = (unsigned char*)shaderCode->GetBufferPointer();
	unsigned int bufferSize = (unsigned int)shaderCode->GetBufferSize();

	ShaderReflection(pBuffer, bufferSize);

	// Strip everything not necessary to use the shader
	ID3DBlob* strippedCode = 0;
	HR(D3DStripShader(pBuffer, bufferSize, D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS, &strippedCode))
	m_ShaderByteCode.resize((unsigned)strippedCode->GetBufferSize());
	memcpy(&m_ShaderByteCode[0], strippedCode->GetBufferPointer(), m_ShaderByteCode.size());

	SafeRelease(strippedCode);
	SafeRelease(shaderCode);
	SafeRelease(errorMsgs);

	return true;
}

void ShaderVariation::ShaderReflection(unsigned char* pBuffer, unsigned bufferSize)
{
	ID3D11ShaderReflection* reflection = 0;
	D3D11_SHADER_DESC shaderDesc;

	D3DReflect(pBuffer, bufferSize, IID_ID3D11ShaderReflection, (void**)&reflection);
	reflection->GetDesc(&shaderDesc);

	map<string, unsigned> cbRegisterMap;

	for (unsigned i = 0; i < shaderDesc.BoundResources; ++i)
	{
		D3D11_SHADER_INPUT_BIND_DESC resourceDesc;
		reflection->GetResourceBindingDesc(i, &resourceDesc);
		string resourceName(resourceDesc.Name);
		if (resourceDesc.Type == D3D_SIT_CBUFFER)
			cbRegisterMap[resourceName] = resourceDesc.BindPoint;
	}

	for (unsigned int c = 0; c < shaderDesc.ConstantBuffers; ++c)
	{
		ID3D11ShaderReflectionConstantBuffer* pConstantBuffer = reflection->GetConstantBufferByIndex(c);
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		pConstantBuffer->GetDesc(&bufferDesc);
		unsigned cbRegister = cbRegisterMap[string(bufferDesc.Name)];

		for (unsigned v = 0; v < bufferDesc.Variables; ++v)
		{
			ID3D11ShaderReflectionVariable* pVariable = pConstantBuffer->GetVariableByIndex(v);
			D3D11_SHADER_VARIABLE_DESC variableDesc;
			pVariable->GetDesc(&variableDesc);

			ShaderParameter parameter;
			parameter.Name = variableDesc.Name;
			parameter.Offset = variableDesc.StartOffset;
			parameter.Type = m_ShaderType;
			parameter.Size = variableDesc.Size;
			parameter.Buffer = cbRegister;
			m_ShaderParameters[parameter.Name] = parameter;
		}
	}
	SafeRelease(reflection);
}

void ShaderVariation::SetDefines(const vector<string>& defines)
{
	m_Defines = defines;
}
