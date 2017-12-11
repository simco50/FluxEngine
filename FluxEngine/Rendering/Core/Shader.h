#pragma once

enum class ShaderType;

class Graphics;
class ShaderVariation;
class IFile;

class Shader
{
public:
	Shader(Graphics* pGraphics);
	~Shader();

	DELETE_COPY(Shader)

	bool Load(const string& filePath);
	ShaderVariation* GetVariation(const ShaderType type, const string& defines = string(""));
	const string& GetSource() { return m_ShaderSource; }

	static string GetEntryPoint(const ShaderType type);

	const string& GetName() const { return m_ShaderName; }

private:
	string MakeSearchHash(const ShaderType type, const string& defines);

	string m_FileDir;
	string m_ShaderName;

	bool ProcessSource(const unique_ptr<IFile>& pFile, stringstream& output);

	string m_ShaderSource;
	using ShaderVariationHash = string;
	map<ShaderVariationHash, unique_ptr<ShaderVariation>> m_ShaderCache;

	Graphics* m_pGraphics;
};