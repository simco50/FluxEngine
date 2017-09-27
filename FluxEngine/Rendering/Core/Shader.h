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

	bool Load(const string& filePath);
	ShaderVariation* GetVariation(ShaderType type, const vector<string>& defines = {});
	const string& GetSource(ShaderType type) const;

private:
	string m_FileDir;

	bool ProcessSource(ifstream& stream, string& output);
	void CommentFunction(string& input, const string& function);

	string m_VertexShaderSource;
	string m_PixelShaderSource;

	Graphics* m_pGraphics;
};