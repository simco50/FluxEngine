#include "FluxEngine.h"
#include "Shader.h"
#include "ShaderVariation.h"
#include "Graphics.h"
#include "IO\InputStream.h"

Shader::Shader(Context* pContext) :
	Resource(pContext)
{
	m_pGraphics = pContext->GetSubsystem<Graphics>();
}

Shader::~Shader()
{
}

bool Shader::Load(InputStream& inputStream)
{
	std::string fileName = inputStream.GetSource();
	m_ShaderName = Paths::GetFileNameWithoutExtension(fileName);
	m_FileDir = Paths::GetDirectoryPath(fileName);

	AUTOPROFILE_DESC(Shader_Load, m_ShaderName);
	{
		AUTOPROFILE(Shader_ProcessSource);
		std::stringstream codeStream;
		if (!ProcessSource(&inputStream, codeStream))
			return false;

		m_ShaderSource = codeStream.str();
	}

	Reload();

	return true;
}

bool Shader::Reload()
{
	AUTOPROFILE_DESC(Shader_Load, m_ShaderName);
	//Reload all shaders in the cache
	for (auto& p : m_ShaderCache)
	{
		if (p.second == nullptr)
			continue;
		p.second->Create(m_pGraphics);
	}
	return true;
}

ShaderVariation* Shader::GetVariation(const ShaderType type, const std::string& defines)
{
	std::hash<std::string> hasher;
	size_t hash = hasher(defines + (char)type);

	auto pIt = m_ShaderCache.find(hash);
	if (pIt != m_ShaderCache.end())
		return pIt->second.get();

	std::unique_ptr<ShaderVariation> pVariation = std::make_unique<ShaderVariation>(this, type);
	pVariation->SetDefines(defines);
	if (!pVariation->Create(m_pGraphics))
	{
		FLUX_LOG(Warning, "[Shader::GetVariation()] > Failed to load shader variation");
		return nullptr;
	}

	m_ShaderCache[hash] = std::move(pVariation);
	return m_ShaderCache[hash].get();
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

std::string Shader::MakeSearchHash(const ShaderType type, const std::string& defines)
{
	return Printf("TYPE_%i%s", type, defines.c_str());
}

bool Shader::ProcessSource(InputStream* pInputStream, std::stringstream& output)
{
	std::string line;
	while (pInputStream->GetLine(line))
	{
		if (line.substr(0, 8) == "#include")
		{
			std::string includeFilePath = line.substr(9);
			includeFilePath.erase(includeFilePath.begin());
			includeFilePath.pop_back();

			std::unique_ptr<File> newFile = FileSystem::GetFile(m_FileDir + includeFilePath);
			if (newFile == nullptr)
				return false;
			if (!newFile->Open(FileMode::Read, ContentType::Text))
				return false;

			if (!ProcessSource(newFile.get(), output))
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
