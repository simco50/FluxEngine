#pragma once
#include "GraphicsObject.h"
class Shader;
class ConstantBuffer;

enum class ShaderType;

struct ShaderParameter
{
	std::string Name;
	int Buffer = 0;
	int Size = 0;
	int Offset = 0;
	ConstantBuffer* pBuffer = nullptr;
};

class ShaderVariation : public GraphicsObject
{
public:
	ShaderVariation(Graphics* pGraphics, Shader* pOwner, ShaderType type);
	~ShaderVariation();

	bool Create();
	void Release();
	void AddDefine(const std::string& define);
	void SetDefines(const std::string& defines);

	bool SaveToCache(OutputStream& outputStream) const;
	bool LoadFromCache(InputStream& inputStream);

	const std::map<StringHash, ShaderParameter>& GetParameters() const { return m_ShaderParameters; }
	const std::array<ConstantBuffer*, (unsigned int)ShaderParameterType::MAX>& GetConstantBuffers() const { return m_ConstantBuffers; }

	const std::vector<char>& GetByteCode() const { return m_ShaderByteCode; }

	const std::string& GetName() const { return m_Name; }
	bool CreateShader(Graphics* pGraphics, ShaderType type);

private:
	static const int SHADER_CACHE_VERSION = 5;

	bool Compile(Graphics* pGraphics);

	void ShaderReflection(char* pBuffer, unsigned bufferSize, Graphics* pGraphics);

	Shader* m_pParentShader;
	ShaderType m_ShaderType;

	std::string m_Name;
	std::vector<std::string> m_Defines;
	std::vector<char> m_ShaderByteCode;
	std::array<size_t, (size_t)ShaderParameterType::MAX> m_ConstantBufferSizes = {};

	std::map<StringHash, ShaderParameter> m_ShaderParameters;
	std::array<ConstantBuffer*, (unsigned int)ShaderParameterType::MAX> m_ConstantBuffers = {};
};