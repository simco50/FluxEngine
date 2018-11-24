#include "FluxEngine.h"
#include "Shader.h"
#include "ShaderVariation.h"
#include "IO\InputStream.h"
#include "Content\ResourceManager.h"
#include "Graphics.h"
#include "Core\CommandLine.h"

#define USE_SHADER_LINE_DIRECTIVE

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

	std::string fileName = inputStream.GetSource();
	m_Name = Paths::GetFileNameWithoutExtension(fileName);

	std::vector<std::string> dependencies;

	m_LastModifiedTimestamp = 0;
	{
		AUTOPROFILE(Shader_ProcessSource);
		std::stringstream codeStream;
		std::vector<size_t> processedIncludes;
		if (!ProcessSource(inputStream, codeStream, processedIncludes, dependencies))
		{
			return false;
		}

		m_ShaderSource = codeStream.str();
	}

	if (ReloadVariations() == false)
	{
		return false;
	}

	RefreshMemoryUsage();

	ResourceManager* pResourceManager = GetSubsystem<ResourceManager>();
	pResourceManager->ResetDependencies(this);
	for (const std::string& dependency : dependencies)
	{
		pResourceManager->AddResourceDependency(this, dependency);
	}

	return true;
}

bool Shader::ReloadVariations()
{
	AUTOPROFILE_DESC(ReloadVariations, m_Name);

	bool success = true;

	//Reload all shaders in the cache
	for (auto& map : m_ShaderCache)
	{
		for (auto& p : map)
		{
			if (p.second == nullptr)
			{
				continue;
			}
			success = p.second->Create() ? success : false;
		}
	}
	return success;
}

ShaderVariation* Shader::GetOrCreateVariation(ShaderType type, const std::string& defines)
{
	AUTOPROFILE(Shader_GetOrCreateVariation);

	size_t hash = HashString(defines);
	auto pIt = m_ShaderCache[(size_t)type].find(hash);
	if (pIt != m_ShaderCache[(size_t)type].end())
	{
		return pIt->second.get();
	}

	Graphics* pGraphics = m_pContext->GetSubsystem<Graphics>();
	std::unique_ptr<ShaderVariation> pVariation = std::make_unique<ShaderVariation>(pGraphics, this, type);

	std::stringstream cacheName;
	cacheName << m_Name + Shader::GetEntryPoint(type) << "_" << hash;
	if (CommandLine::GetBool("NoShaderCache") == false && pVariation->LoadFromCache(cacheName.str()))
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
		if(pVariation->SaveToCache(cacheName.str()) == false)
		{
			FLUX_LOG(Warning, "[Shader::GetVariation()] > Failed to save shader variation to cache");
		}
	}

	m_ShaderCache[(size_t)type][hash] = std::move(pVariation);
	return m_ShaderCache[(size_t)type][hash].get();
}

const char* Shader::GetEntryPoint(ShaderType type)
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

bool Shader::ProcessSource(InputStream& inputStream, std::stringstream& output, std::vector<size_t>& processedIncludes, std::vector<std::string>& dependencies)
{
	if (GetFilePath() != inputStream.GetSource())
	{
		dependencies.push_back(inputStream.GetSource());
	}

	const DateTime timestamp = FileSystem::GetLastModifiedTime(inputStream.GetSource());
	if (timestamp > m_LastModifiedTimestamp)
	{
		m_LastModifiedTimestamp = timestamp;
	}

	std::string line;

	int linesProcessed = 0;
	bool placedLineDirective = false;
	while (inputStream.GetLine(line))
	{
		if (line.substr(0, 8) == "#include")
		{
			std::string includeFilePath = std::string(line.begin() + 10, line.end() - 1);
			size_t includeHash = HashString(includeFilePath);
			if (std::find(processedIncludes.begin(), processedIncludes.end(), includeHash) == processedIncludes.end())
			{
				processedIncludes.push_back(includeHash);
				std::string basePath = Paths::GetDirectoryPath(inputStream.GetSource());
				std::string filePath = basePath + includeFilePath;
				std::unique_ptr<File> newFile = FileSystem::GetFile(filePath);
				if (newFile == nullptr)
				{
					FLUX_LOG(Warning, "[Shader::ProcessSource] Failed to find include file '%s'", filePath.c_str());
					return false;
				}
				if (!newFile->OpenRead())
				{
					return false;
				}

				if (!ProcessSource(*newFile, output, processedIncludes, dependencies))
				{
					return false;
				}
			}
			placedLineDirective = false;
		}
		else
		{
			if (placedLineDirective == false)
			{
				placedLineDirective = true;
#ifdef USE_SHADER_LINE_DIRECTIVE
				output << "#line " << linesProcessed + 1 << " \"" << inputStream.GetSource() << "\"\n";
#endif
			}
			output << line << '\n';
		}
		++linesProcessed;
	}
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
