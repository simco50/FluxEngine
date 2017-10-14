#pragma once

class Graphics;
class ShaderVariation;
class IFile;

enum class ShaderType : unsigned char
{
	VertexShader	= 0,
	PixelShader		= 1 << 0,
	//#todo Geometry shader
};

class Shader
{
public:
	Shader(Graphics* pGraphics);
	~Shader();

	DELETE_COPY(Shader)

	bool Load(const string& filePath);
	ShaderVariation* GetVariation(const ShaderType type, const string& defines = string(""));
	const string& GetSource(const ShaderType type) const;

private:
	string m_FileDir;
	string m_ShaderName;

	bool ProcessSource(const unique_ptr<IFile>& pFile, stringstream& output);
	void StripFunction(const string& input, string& out, const string& function);

	string m_VertexShaderSource;
	string m_PixelShaderSource;

	map<string, ShaderVariation*> m_VertexShaderCache;
	map<string, ShaderVariation*> m_PixelShaderCache;

	Graphics* m_pGraphics;
};