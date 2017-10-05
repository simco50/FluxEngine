#include "stdafx.h"
#include "Shader.h"
#include "ShaderVariation.h"
#include "Graphics.h"

Shader::Shader(Graphics* pGraphics) :
	m_pGraphics(pGraphics)
{

}

Shader::~Shader()
{
	for (auto p : m_VertexShaderCache)
		SafeDelete(p.second);
	m_VertexShaderCache.clear();
	for (auto p : m_PixelShaderCache)
		SafeDelete(p.second);
	m_PixelShaderCache.clear();
}

bool Shader::Load(const string& filePath)
{
	unsigned int slashIdx = (unsigned int)filePath.rfind('/') + 1;
	m_FileDir = filePath.substr(0, slashIdx);
	m_ShaderName = filePath.substr(slashIdx, filePath.rfind('.') - slashIdx);

	ifstream stream;
	stream.open(filePath);
	if (stream.fail())
	{
		FLUX_LOG(WARNING, "Failed to load file: '%s'", filePath.c_str());
		return false;
	}
	string code;
	if (!ProcessSource(stream, code))
		return false;

	m_VertexShaderSource = code;
	m_PixelShaderSource = code;

	CommentFunction(m_VertexShaderSource, "void PS(");
	CommentFunction(m_PixelShaderSource, "void VS(");

	return true;
}

ShaderVariation* Shader::GetVariation(const ShaderType type, const string& defines)
{
	switch (type)
	{
	case ShaderType::VertexShader:
		for (auto p : m_VertexShaderCache)
		{
			if (p.first == defines)
				return p.second;
		}
		break;
	case ShaderType::PixelShader:
		for (auto p : m_PixelShaderCache)
		{
			if (p.first == defines)
				return p.second;
		}
		break;
	default:
		break;
	}

	ShaderVariation* pVariation = new ShaderVariation(this, type);
	pVariation->SetDefines(defines);
	if (!pVariation->Create(m_pGraphics))
	{
		FLUX_LOG(ERROR, "[Shader::GetVariation()] > Failed to load shader variation");
		return nullptr;
	}

	switch (type)
	{
	case ShaderType::VertexShader:
		m_VertexShaderCache[defines] = pVariation;
		break;
	case ShaderType::PixelShader:
		m_PixelShaderCache[defines] = pVariation;
		break;
	}

	return pVariation;
}

const std::string& Shader::GetSource(const ShaderType type) const
{
	switch (type)
	{
	case ShaderType::VertexShader:
		return m_VertexShaderSource;
		break;
	case ShaderType::PixelShader:
		return m_PixelShaderSource;
		break;
	default:
		break;
	}
	throw;
}

bool Shader::ProcessSource(ifstream& stream, string& output)
{
	AUTOPROFILE_DESC(Shader_ProcessSource, m_ShaderName);

	string line;
	while (getline(stream, line))
	{
		if (line.substr(0, 8) == "#include")
		{
			string includeFilePath = line.substr(9);
			includeFilePath.erase(includeFilePath.begin());
			includeFilePath.pop_back();
			ifstream newStream(m_FileDir + includeFilePath);
			if (newStream.fail())
				return false;

			if(!ProcessSource(newStream, output))
				return false;
		}
		else
		{
			output += line;
			output += "\n";
		}
	}
	output += "\n";
	stream.close();
	return true;
}

void Shader::CommentFunction(string& input, const string& function)
{
	size_t startPos = input.find(function);
	if (startPos == string::npos)
		return;
	input.insert(startPos, "/*");

	int braceCount = 0;

	for (size_t i = startPos + function.size(); i < input.size(); ++i)
	{
		if (input[i] == '{')
		{
			++braceCount;
			continue;
		}
		if (input[i] == '}')
		{
			--braceCount;
			if (braceCount == 0)
			{
				input.insert(i + 1, "*/");
				break;
			}
			continue;
		}
	}
}
