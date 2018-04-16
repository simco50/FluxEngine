#pragma once
#include "GraphicsDefines.h"
#include "Core\Object.h"
class Shader;
class ConstantBuffer;
class Graphics;

enum class ShaderType;

struct ShaderParameter
{
	ShaderType Type;
	std::string Name;
	int Buffer;
	int Size;
	int Offset;
	ConstantBuffer* pBuffer;
};

class ShaderVariation : public Object
{
	FLUX_OBJECT(ShaderVariation, Object)

public:
	ShaderVariation(Context* pContext, Shader* pOwner, const ShaderType type);
	~ShaderVariation();

	bool Create();
	void Release();
	void AddDefine(const std::string& define);
	void SetDefines(const std::string& defines);

	const std::map<std::string, ShaderParameter>& GetParameters() const { return m_ShaderParameters; }
	const std::array<ConstantBuffer*, (unsigned int)ShaderParameterType::MAX>& GetConstantBuffers() const { return m_ConstantBuffers; }

	const void* GetShaderObject() const { return m_pShaderObject; }
	const std::vector<char>& GetByteCode() const { return m_ShaderByteCode; }

	const std::string& GetName() const { return m_Name; }

private:
	bool Compile(Graphics* pGraphics);

	void ShaderReflection(char* pBuffer, unsigned bufferSize, Graphics* pGraphics);

	Shader* m_pParentShader;
	ShaderType m_ShaderType;
	void* m_pShaderObject = nullptr;

	std::string m_Name;
	std::vector<std::string> m_Defines;
	std::vector<char> m_ShaderByteCode;

	std::map<std::string, ShaderParameter> m_ShaderParameters;
	std::array<ConstantBuffer*, (unsigned int)ShaderParameterType::MAX> m_ConstantBuffers = {};
};