#pragma once

class Graphics;
class ShaderVariation;

enum class ShaderType
{
	VertexShader,
	PixelShader,
};

class Shader
{
public:
	Shader(Graphics* pGraphics);
	~Shader();

	DELETE_COPY(Shader)

	bool Load(const string& filePath);
	ShaderVariation* GetVariation(ShaderType type, const string& defines = string(""));
	const string& GetSource(ShaderType type) const;

private:
	string m_FileDir;

	bool ProcessSource(ifstream& stream, string& output);
	void CommentFunction(string& input, const string& function);

	string m_VertexShaderSource;
	string m_PixelShaderSource;

	map<string, ShaderVariation*> m_VertexShaderCache;
	map<string, ShaderVariation*> m_PixelShaderCache;

	Graphics* m_pGraphics;
};