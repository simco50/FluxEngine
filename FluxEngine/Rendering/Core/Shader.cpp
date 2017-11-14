#include "stdafx.h"
#include "Shader.h"
#include "ShaderVariation.h"
#include "Graphics.h"
#include "FileSystem\File\PhysicalFile.h"

Shader::Shader(Graphics* pGraphics) :
	m_pGraphics(pGraphics)
{

}

Shader::~Shader()
{
}

bool Shader::Load(const string& filePath)
{
	m_ShaderName = Paths::GetFileNameWithoutExtension(filePath);
	m_FileDir = Paths::GetDirectoryPath(filePath);

	AUTOPROFILE_DESC(Shader_Load, m_ShaderName);

	unique_ptr<IFile> pPtr = FileSystem::GetFile(filePath);
	if (!pPtr->Open(FileMode::Read, ContentType::Text))
	{
		FLUX_LOG(WARNING, "Failed to load file: '%s'", filePath.c_str());
		return false;
	}

	{
		AUTOPROFILE(Shader_ProcessSource);
		stringstream codeStream;
		if (!ProcessSource(std::move(pPtr), codeStream))
			return false;

		m_ShaderSource = codeStream.str();
	}

	return true;
}

ShaderVariation* Shader::GetVariation(const ShaderType type, const string& defines)
{
	ShaderVariationHash searchKey = MakeSearchHash(type, defines);

	auto pShader = m_ShaderCache.find(searchKey);
	if (pShader != m_ShaderCache.end())
		return pShader->second.get();

	unique_ptr<ShaderVariation> pVariation = make_unique<ShaderVariation>(this, type);
	pVariation->SetDefines(defines);
	if (!pVariation->Create(m_pGraphics))
	{
		FLUX_LOG(ERROR, "[Shader::GetVariation()] > Failed to load shader variation");
		return nullptr;
	}

	m_ShaderCache[searchKey] = std::move(pVariation);

	return m_ShaderCache[searchKey].get();
}

std::string Shader::GetEntryPoint(const ShaderType type)
{
	switch (type)
	{
	case ShaderType::VertexShader:
		return "VSMain";
	case ShaderType::PixelShader:
		return "PSMain";
	case ShaderType::GeometryShader:
		return "GSMain";
	case ShaderType::ComputeShader:
		return "CSMain";
	default:
		return "";
	}
}

std::string Shader::MakeSearchHash(const ShaderType type, const string& defines)
{
	std::stringstream str;
	str << "TYPE_" << (int)type << defines;
	return str.str();
}

bool Shader::ProcessSource(const unique_ptr<IFile>& pFile, stringstream& output)
{
	string line;
	while (pFile->GetLine(line))
	{
		if (line.substr(0, 8) == "#include")
		{
			string includeFilePath = line.substr(9);
			includeFilePath.erase(includeFilePath.begin());
			includeFilePath.pop_back();

			unique_ptr<IFile> newFile = FileSystem::GetFile(m_FileDir + includeFilePath);
			if (newFile == nullptr)
				return false;
			if (!newFile->Open(FileMode::Read, ContentType::Text))
				return false;

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