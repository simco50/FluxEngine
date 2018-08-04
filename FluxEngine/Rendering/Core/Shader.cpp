#include "FluxEngine.h"
#include "Shader.h"
#include "ShaderVariation.h"
#include "IO\InputStream.h"
#include "Content\ResourceManager.h"
#include "Graphics.h"

Shader::Shader(Context* pContext) :
	Resource(pContext)
{
}

Shader::~Shader()
{
}

bool Shader::Load(InputStream& inputStream)
{
	AUTOPROFILE_DESC(Shader_Load, m_Name);
	GetSubsystem<ResourceManager>()->ResetDependencies(this);

	std::string fileName = inputStream.GetSource();
	m_Name = Paths::GetFileNameWithoutExtension(fileName);

	m_LastModifiedTimestamp = 0;
	{
		AUTOPROFILE(Shader_ProcessSource);
		std::stringstream codeStream;
		std::vector<size_t> processedIncludes;
		if (!ProcessSource(inputStream, codeStream, processedIncludes))
			return false;

		m_ShaderSource = codeStream.str();
	}

	ReloadVariations();

	RefreshMemoryUsage();

	return true;
}

bool Shader::ReloadVariations()
{
	AUTOPROFILE_DESC(ReloadVariations, m_Name);
	//Reload all shaders in the cache
	for (auto& map : m_ShaderCache)
	{
		for (auto& p : map)
		{
			if (p.second == nullptr)
				continue;
			p.second->Create();
		}
	}
	return true;
}

ShaderVariation* Shader::GetOrCreateVariation(const ShaderType type, const std::string& defines)
{
	AUTOPROFILE(Shader_GetOrCreateVariation);

	size_t hash = std::hash<std::string>{}(defines);
	auto pIt = m_ShaderCache[(size_t)type].find(hash);
	if (pIt != m_ShaderCache[(size_t)type].end())
		return pIt->second.get();

	std::unique_ptr<ShaderVariation> pVariation = std::make_unique<ShaderVariation>(m_pContext, this, type);
	
	std::stringstream cacheName;
	cacheName << m_Name + Shader::GetEntryPoint(type) << "_" << hash;
	if (pVariation->LoadFromCache(cacheName.str()))
	{
		if (!pVariation->CreateShader(GetSubsystem<Graphics>(), type))
		{
			return nullptr;
		}
		m_ShaderCache[(size_t)type][hash] = std::move(pVariation);
		return m_ShaderCache[(size_t)type][hash].get();
	}
	else
	{
		pVariation->SetDefines(defines);
		if (!pVariation->Create())
		{
			FLUX_LOG(Warning, "[Shader::GetVariation()] > Failed to load shader variation");
			return nullptr;
		}
		pVariation->SaveToCache(cacheName.str());
	}

	m_ShaderCache[(size_t)type][hash] = std::move(pVariation);
	return m_ShaderCache[(size_t)type][hash].get();
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

bool Shader::ProcessSource(InputStream& inputStream, std::stringstream& output, std::vector<size_t>& processedIncludes)
{
	ResourceManager* pResourceManager = GetSubsystem<ResourceManager>();
	if(GetFilePath() != inputStream.GetSource())
		pResourceManager->AddResourceDependency(this, inputStream.GetSource());

	DateTime timestamp = FileSystem::GetLastModifiedTime(inputStream.GetSource());
	if (timestamp > m_LastModifiedTimestamp)
	{
		m_LastModifiedTimestamp = timestamp;
	}

	std::string line;
	while (inputStream.GetLine(line))
	{
		if (line.substr(0, 8) == "#include")
		{
			std::string includeFilePath = std::string(line.begin() + 10, line.end() - 1);
			size_t includeHash = std::hash<std::string>{}(includeFilePath);
			if (std::find(processedIncludes.begin(), processedIncludes.end(), includeHash) != processedIncludes.end())
				continue;
			processedIncludes.push_back(includeHash);
			std::string basePath = Paths::GetDirectoryPath(inputStream.GetSource());
			std::unique_ptr<File> newFile = FileSystem::GetFile(basePath + includeFilePath);
			if (newFile == nullptr)
				return false;
			if (!newFile->OpenRead())
				return false;
			if (!ProcessSource(*newFile, output, processedIncludes))
				return false;
		}
		else
		{
			output << line << '\n';
		}
	}
	output << '\n';
	return true;
}

void Shader::RefreshMemoryUsage()
{
	unsigned int memoryUsage = 0;
	memoryUsage += sizeof(Shader);
	memoryUsage += (unsigned int)m_ShaderCache.size() * sizeof(ShaderVariation);
	memoryUsage += (unsigned int)m_ShaderSource.size();
	SetMemoryUsage(memoryUsage);
}
