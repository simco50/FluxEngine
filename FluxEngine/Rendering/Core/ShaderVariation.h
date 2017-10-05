#pragma once
class Shader;
class ConstantBuffer;
class Graphics;

enum class ShaderType : unsigned char;

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

	void SetParameter(const string& name, void* pValue);

	const map<string, ShaderParameter>& GetParameters() const { return m_ShaderParameters; }
	const vector<unique_ptr<ConstantBuffer>>& GetConstantBuffers() const { return m_ConstantBuffers; }

	void* const GetShaderObject() const { return m_pShaderObject; }
	const vector<unsigned char>& GetByteCode() const { return m_ShaderByteCode; }

private:
	bool Compile(Graphics* pGraphics);
	void ShaderReflection(unsigned char* pBuffer, unsigned bufferSize, Graphics* pGraphics);

	Shader* m_pParentShader;
	ShaderType m_ShaderType;

	void* m_pShaderObject = nullptr;

	vector<string> m_Defines;
	vector<unsigned char> m_ShaderByteCode;

	map<string, ShaderParameter> m_ShaderParameters;
	vector<unique_ptr<ConstantBuffer>> m_ConstantBuffers;
};