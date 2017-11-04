#pragma once
#include "GraphicsDefines.h"
class Shader;
class ConstantBuffer;
class Graphics;

enum class ShaderType : unsigned int;

struct ShaderParameter
{
	ShaderType Type;
	string Name;
	int Buffer;
	int Size;
	int Offset;
	ConstantBuffer* pBuffer;
};

class ShaderVariation
{
public:
	ShaderVariation(Shader* pOwner, const ShaderType type);
	~ShaderVariation();

	bool Create(Graphics* pGraphics);
	void Release();
	void SetDefines(const string& defines);

	template<typename T>
	void SetParameter(const std::string& name, const T& value);

	const map<string, ShaderParameter>& GetParameters() const { return m_ShaderParameters; }
	const array<ConstantBuffer*, (unsigned int)ShaderParameterType::MAX>& GetConstantBuffers() const { return m_ConstantBuffers; }

	void* const GetShaderObject() const { return m_pShaderObject; }
	const vector<char>& GetByteCode() const { return m_ShaderByteCode; }

private:
	bool Compile(Graphics* pGraphics);

	void ShaderReflection(char* pBuffer, unsigned bufferSize, Graphics* pGraphics);

	Shader* m_pParentShader;
	ShaderType m_ShaderType;

	void* m_pShaderObject = nullptr;

	vector<string> m_Defines;
	vector<char> m_ShaderByteCode;

	map<string, ShaderParameter> m_ShaderParameters;
	array<ConstantBuffer*, (unsigned int)ShaderParameterType::MAX> m_ConstantBuffers = {};
};

template<typename T>
void ShaderVariation::SetParameter(const std::string& name, const T& value)
{
	auto pParameter = m_ShaderParameters.find(name);
	if (pParameter == m_ShaderParameters.end())
		return;
#ifdef _DEBUG
	if (sizeof(T) != pParameter->second.Size)
	{
		FLUX_LOG(ERROR, "[ShaderVariation::SetParameter] > Size mismatch. Parameter '%s' expected value with size '%i' but input value's size is '%i'", name.c_str(), pParameter->second.Size, sizeof(value));
		return;
	}
#endif
	pParameter->second.pBuffer->SetParameter(pParameter->second.Offset, pParameter->second.Size, &value);
}
