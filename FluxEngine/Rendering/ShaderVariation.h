#pragma once
class Shader;
enum class ShaderType;

struct ShaderParameter
{
	ShaderType Type;
	string Name;
	int Buffer;
	int Size;
	int Offset;
};

class ShaderVariation
{
public:
	ShaderVariation(Shader* pOwner, const ShaderType type);
	~ShaderVariation();

	bool Create(ID3D11Device* pDevice);
	void Release();
	void SetDefines(const vector<string>& defines);

	const map<string, ShaderParameter>& GetParameters() const { return m_ShaderParameters; }

	void* const GetShaderObject() const { return m_pShaderObject; }

private:
	bool Compile();
	void ShaderReflection(unsigned char* pBuffer, unsigned bufferSize);

	Shader* m_pParentShader;
	ShaderType m_ShaderType;

	void* m_pShaderObject = nullptr;

	vector<string> m_Defines;
	string m_ShaderByteCode;

	map<string, ShaderParameter> m_ShaderParameters;
};

