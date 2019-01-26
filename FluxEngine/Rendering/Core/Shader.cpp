#include "FluxEngine.h"
#include "Shader.h"
#include "ShaderVariation.h"
#include "IO\InputStream.h"
#include "Content\ResourceManager.h"
#include "Graphics.h"
#include "Core\CommandLine.h"
#include "FileSystem\File\PhysicalFile.h"

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

	m_LastModifiedTimestamp = DateTime(0);

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
			if (m_ShaderSource.length() == 0)
			{
				LoadSource();
			}
			success = p.second->Create() ? success : false;
		}
	}
	return success;
}

ShaderVariation* Shader::GetOrCreateVariation(ShaderType type, const std::string& defines)
{
	AUTOPROFILE(Shader_GetOrCreateVariation);

	StringHash hash(defines);
	auto pIt = m_ShaderCache[(size_t)type].find(hash);
	if (pIt != m_ShaderCache[(size_t)type].end())
	{
		return pIt->second.get();
	}

	Graphics* pGraphics = GetSubsystem<Graphics>();

	std::stringstream cacheName;
	cacheName << m_Name + Shader::GetEntryPoint(type) << "_" << hash;
	std::string filePath = Printf("%s%s.bin", Paths::ShaderCacheDir().c_str(), cacheName.str().c_str());

	std::unique_ptr<ShaderVariation> pVariation = std::make_unique<ShaderVariation>(pGraphics, this, type);

	if (TryLoadFromCache(filePath, pVariation))
	{
		if (pVariation->CreateShader(pGraphics, type) == false)
		{
			FLUX_LOG(Warning, "[Shader::GetOrCreateVariation] Failed to load shader from the shader cache");
			return nullptr;
		}
	}
	else
	{
		if (m_ShaderSource.length() == 0)
		{
			LoadSource();
		}

		pVariation->SetDefines(defines);
		if (!pVariation->Create())
		{
			FLUX_LOG(Warning, "[Shader::GetVariation()] > Failed to load shader variation");
			return nullptr;
		}

		if (TrySaveToCache(filePath, pVariation) == false)
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
#ifdef SHADER_GEOMETRY_ENABLE
	case ShaderType::GeometryShader:
		return "GSMain";
#endif
#ifdef SHADER_COMPUTE_ENABLE
	case ShaderType::ComputeShader:
		return "CSMain";
#endif
#ifdef SHADER_TESSELLATION_ENABLE
	case ShaderType::DomainShader:
		return "DSMain";
	case ShaderType::HullShader:
		return "HSMain";
#endif
	default:
		return "";
	}
}

bool Shader::ProcessSource(InputStream& inputStream, std::stringstream& output, std::vector<StringHash>& processedIncludes, std::vector<std::string>& dependencies)
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
			StringHash includeHash(includeFilePath);
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

bool Shader::LoadSource()
{
	AUTOPROFILE_DESC(Shader_LoadSource, m_Name);
	ResourceManager* pResourceManager = GetSubsystem<ResourceManager>();
	pResourceManager->ResetDependencies(this);

	std::vector<std::string> dependencies;
	std::unique_ptr<File> pFile = FileSystem::GetFile(m_FilePath);
	if (pFile == nullptr)
	{
		FLUX_LOG(Warning, "[Shader::GetOrCreateVariation() Failed to file file at '%s']", m_FilePath.c_str());
		return false;
	}
	if (!pFile->OpenRead())
	{
		FLUX_LOG(Warning, "[Shader::GetOrCreateVariation() Failed to open file at '%s']", m_FilePath.c_str());
		return false;
	}
	std::stringstream codeStream;
	std::vector<StringHash> processedIncludes;
	ProcessSource(*pFile, codeStream, processedIncludes, dependencies);
	m_ShaderSource = codeStream.str();

	for (const std::string& dep : dependencies)
	{
		pResourceManager->AddResourceDependency(this, dep);
	}

	return true;
}

bool Shader::TryLoadFromCache(const std::string& path, std::unique_ptr<ShaderVariation>& pVariation)
{
	if (CommandLine::GetBool("NoShaderCache"))
	{
		return false;
	}

	DateTime shaderTime = FileSystem::GetLastModifiedTime(path);
	if (shaderTime == DateTime(0) || shaderTime < m_LastModifiedTimestamp)
	{
		return false;
	}
	std::unique_ptr<PhysicalFile> pFile = std::make_unique<PhysicalFile>(path);
	if (pFile->OpenRead() == false)
	{
		return false;
	}

	return pVariation->LoadFromCache(*pFile);
}

bool Shader::TrySaveToCache(const std::string& path, std::unique_ptr<ShaderVariation>& pVariation)
{
	std::unique_ptr<PhysicalFile> pFile = std::make_unique<PhysicalFile>(path);
	if (pFile->OpenWrite() == false)
	{
		return false;
	}
	return pVariation->SaveToCache(*pFile);
}

void Shader::RefreshMemoryUsage()
{
	unsigned int memoryUsage = 0;
	memoryUsage += sizeof(Shader);
	memoryUsage += (unsigned int)m_ShaderCache.size() * sizeof(ShaderVariation);
	memoryUsage += (unsigned int)m_ShaderSource.size();
	SetMemoryUsage(memoryUsage);
}
