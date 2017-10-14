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
	AUTOPROFILE(Shader_Load);

	unsigned int slashIdx = (unsigned int)filePath.rfind('/') + 1;
	m_FileDir = filePath.substr(0, slashIdx);
	m_ShaderName = filePath.substr(slashIdx, filePath.rfind('.') - slashIdx);

	unique_ptr<IFile> pPtr = FileSystem::GetFile(filePath);
	if (!pPtr->Open(FileMode::Read, ContentType::Text))
	{
		FLUX_LOG(WARNING, "Failed to load file: '%s'", filePath.c_str());
		return false;
	}

	stringstream codeStream;
	if (!ProcessSource(std::move(pPtr), codeStream))
		return false;

	std::string code = codeStream.str();
	StripFunction(code, m_VertexShaderSource, "void PS(");
	StripFunction(code, m_PixelShaderSource, "void VS(");

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

bool Shader::ProcessSource(const unique_ptr<IFile>& pFile, stringstream& output)
{
	AUTOPROFILE_DESC(Shader_ProcessSource, m_ShaderName);

	string line;
	while (pFile->GetLine(line))
	{
		if (line.substr(0, 8) == "#include")
		{
			string includeFilePath = line.substr(9);
			includeFilePath.erase(includeFilePath.begin());
			includeFilePath.pop_back();

			unique_ptr<IFile> newFile = FileSystem::GetFile(m_FileDir + includeFilePath);
			newFile->Open(FileMode::Read, ContentType::Text);

			if(!ProcessSource(std::move(newFile), output))
				return false;
		}
		else
		{
			output << line;
			output << '\n';
		}
	}
	output << '\n';
	return true;
}

void Shader::StripFunction(const string& input, string& out, const string& function)
{
	size_t startCommentPos = input.find(function);
	if (startCommentPos == string::npos)
		out = input;

	size_t endCommentPos = 0;
	int braceCount = 0;
	for (size_t i = startCommentPos + function.size(); i < input.size(); ++i)
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
				endCommentPos = i + 1;
				break;
			}
			continue;
		}
	}
	if (braceCount != 0)
		out = input;

	size_t newSize = input.size() - (endCommentPos - startCommentPos);
	out.resize(newSize);
	memcpy(&out[0], input.data(), startCommentPos);
	memcpy(&out[0] + startCommentPos, input.data() + endCommentPos, input.size() - endCommentPos);
}
