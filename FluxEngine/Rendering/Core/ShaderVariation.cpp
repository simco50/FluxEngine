#include "FluxEngine.h"
#include "ShaderVariation.h"
#include "Shader.h"
#include "Graphics.h"
#include "ConstantBuffer.h"
#include "FileSystem\File\PhysicalFile.h"

ShaderVariation::ShaderVariation(Context* pContext, Shader* pShader, const ShaderType type) :
	Object(pContext),
	m_pParentShader(pShader),
	m_ShaderType(type)
{

}

ShaderVariation::~ShaderVariation()
{
	Release();
}


void ShaderVariation::Release()
{
	SafeRelease(m_pShaderObject);
}

void ShaderVariation::AddDefine(const std::string& define)
{
	m_Defines.push_back(define);
}

void ShaderVariation::SetDefines(const std::string& defines)
{
	std::stringstream stream(defines);
	std::string define;
	while (std::getline(stream, define, ','))
	{
		m_Defines.push_back(define);
	}
}

bool ShaderVariation::SaveToCache(const std::string& cacheName) const
{
	AUTOPROFILE_DESC(ShaderVariation_SaveToCache, cacheName);

	std::stringstream filePathStream;
	filePathStream << Paths::ShaderCacheDir() << "\\" << cacheName << ".bin";
	std::unique_ptr<PhysicalFile> pFile = std::make_unique<PhysicalFile>(filePathStream.str());
	if (pFile->OpenWrite() == false)
	{
		return false;
	}
	pFile->WriteSizedString("SHDR");
	pFile->WriteInt(SHADER_CACHE_VERSION);
	pFile->WriteSizedString(m_Name);
	pFile->WriteUByte((unsigned char)m_Defines.size());
	for (const std::string& define : m_Defines)
	{
		pFile->WriteSizedString(define);
	}
	pFile->WriteUByte((unsigned char)m_ShaderParameters.size());
	for (const std::pair<std::string, ShaderParameter>& pair : m_ShaderParameters)
	{
		pFile->WriteSizedString(pair.first);
		const ShaderParameter& parameter = pair.second;
		pFile->WriteUByte((unsigned char)parameter.Type);
		pFile->WriteSizedString(parameter.Name);
		pFile->WriteInt(parameter.Buffer);
		pFile->WriteInt(parameter.Size);
		pFile->WriteInt(parameter.Offset);
	}
	for (size_t size : m_ConstantBufferSizes)
	{
		pFile->WriteInt((int)size);
	}
	pFile->WriteInt((int)m_ShaderByteCode.size());
	pFile->Write(m_ShaderByteCode.data(), m_ShaderByteCode.size());
	if (pFile->Close() == false)
	{
		return false;
	}
	return true;
}

bool ShaderVariation::LoadFromCache(const std::string& cacheName)
{
	AUTOPROFILE_DESC(ShaderVariation_LoadFromCache, cacheName);

	std::stringstream filePathStream;
	filePathStream << Paths::ShaderCacheDir() << cacheName << ".bin";
	std::unique_ptr<PhysicalFile> pFile = std::make_unique<PhysicalFile>(filePathStream.str());
	
	DateTime timeStamp = FileSystem::GetLastModifiedTime(filePathStream.str());
	if (timeStamp.Ticks && timeStamp < m_pParentShader->GetLastModifiedTimestamp())
	{
		return false;
	}
	
	if (pFile->OpenRead() == false)
	{
		//Shader is not cached, we have to compile from scratch
		return false;
	}
	std::string id = pFile->ReadSizedString();
	if (id != "SHDR")
	{
		return false;
	}
	int version = pFile->ReadInt();
	if (version != SHADER_CACHE_VERSION)
	{
		FLUX_LOG(Warning, "[ShaderVariation::LoadFromCache()] > Cached shader version mismatch: %i, expected %i", version, SHADER_CACHE_VERSION);
		return false;
	}
	m_Name = pFile->ReadSizedString();
	m_Defines.resize((size_t)pFile->ReadByte());
	for (std::string& define : m_Defines)
	{
		define = pFile->ReadSizedString();
	}
	m_ShaderParameters.clear();
	unsigned char parameterCount = pFile->ReadByte();
	for (unsigned char i = 0; i < parameterCount ; i++)
	{
		std::string parameterName = pFile->ReadSizedString();
		ShaderParameter& parameter = m_ShaderParameters[parameterName];

		parameter.Type = (ShaderType)pFile->ReadUByte();
		parameter.Name = pFile->ReadSizedString();
		parameter.Buffer = pFile->ReadInt();
		parameter.Size = pFile->ReadInt();
		parameter.Offset = pFile->ReadInt();
	}
	for (size_t& size : m_ConstantBufferSizes)
	{
		size = (size_t)pFile->ReadInt();
	}

	m_ShaderByteCode.resize((size_t)pFile->ReadInt());
	pFile->Read(m_ShaderByteCode.data(), m_ShaderByteCode.size());
	if (pFile->Close() == false)
	{
		return false;
	}

	Graphics* pGraphics = GetSubsystem<Graphics>();
	for (size_t i = 0; i < m_ConstantBufferSizes.size() ; i++)
	{
		if (m_ConstantBufferSizes[i] > 0)
			m_ConstantBuffers[i] = pGraphics->GetOrCreateConstantBuffer(ShaderType::VertexShader, (unsigned int)i, (unsigned int)m_ConstantBufferSizes[i]);
		else
			m_ConstantBuffers[i] = nullptr;
	}
	for (auto& pair : m_ShaderParameters)
	{
		pair.second.pBuffer = m_ConstantBuffers[pair.second.Buffer];
	}

	return true;
}