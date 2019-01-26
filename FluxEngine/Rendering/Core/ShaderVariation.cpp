#include "FluxEngine.h"
#include "ShaderVariation.h"
#include "Shader.h"
#include "Graphics.h"
#include "FileSystem\File\PhysicalFile.h"

ShaderVariation::ShaderVariation(Graphics* pGraphics, Shader* pOwner, ShaderType type)
	: GraphicsObject(pGraphics), m_pParentShader(pOwner), m_ShaderType(type)
{

}

ShaderVariation::~ShaderVariation()
{
	Release();
}


void ShaderVariation::Release()
{
	SafeRelease(m_pResource);
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

bool ShaderVariation::SaveToCache(OutputStream& outputStream) const
{
	AUTOPROFILE(ShaderVariation_SaveToCache);

	outputStream.WriteSizedString("SHDR");
	outputStream.WriteInt(SHADER_CACHE_VERSION);
	outputStream.WriteSizedString(m_Name);
	outputStream.WriteUByte((unsigned char)m_ShaderType);
	outputStream.WriteUByte((unsigned char)m_Defines.size());
	for (const std::string& define : m_Defines)
	{
		outputStream.WriteSizedString(define);
	}
	outputStream.WriteUByte((unsigned char)m_ShaderParameters.size());
	for (const auto& pair : m_ShaderParameters)
	{
		outputStream.WriteInt64((int64)pair.first.m_Hash);
		const ShaderParameter& parameter = pair.second;
		outputStream.WriteSizedString(parameter.Name);
		outputStream.WriteInt(parameter.Buffer);
		outputStream.WriteInt(parameter.Size);
		outputStream.WriteInt(parameter.Offset);
	}
	for (size_t size : m_ConstantBufferSizes)
	{
		outputStream.WriteInt((int)size);
	}
	outputStream.WriteInt((int)m_ShaderByteCode.size());
	outputStream.Write(m_ShaderByteCode.data(), m_ShaderByteCode.size());
	return true;
}

bool ShaderVariation::LoadFromCache(InputStream& inputStream)
{
	AUTOPROFILE_DESC(ShaderVariation_LoadFromCache, inputStream.GetSource());
	std::string id = inputStream.ReadSizedString();
	if (id != "SHDR")
	{
		return false;
	}
	int version = inputStream.ReadInt();
	if (version != SHADER_CACHE_VERSION)
	{
		FLUX_LOG(Warning, "[ShaderVariation::LoadFromCache()] > Cached shader version mismatch: %i, expected %i", version, SHADER_CACHE_VERSION);
		return false;
	}
	m_Name = inputStream.ReadSizedString();
	m_ShaderType = (ShaderType)inputStream.ReadUByte();
	m_Defines.resize((size_t)inputStream.ReadByte());
	for (std::string& define : m_Defines)
	{
		define = inputStream.ReadSizedString();
	}
	m_ShaderParameters.clear();
	unsigned char parameterCount = inputStream.ReadByte();
	for (unsigned char i = 0; i < parameterCount; i++)
	{
		StringHash parameterHash = StringHash((size_t)inputStream.ReadInt64());
		ShaderParameter& parameter = m_ShaderParameters[parameterHash];

		parameter.Name = inputStream.ReadSizedString();
		parameter.Buffer = inputStream.ReadInt();
		parameter.Size = inputStream.ReadInt();
		parameter.Offset = inputStream.ReadInt();
	}
	for (size_t& size : m_ConstantBufferSizes)
	{
		size = (size_t)inputStream.ReadInt();
	}

	m_ShaderByteCode.resize((size_t)inputStream.ReadInt());
	inputStream.Read(m_ShaderByteCode.data(), m_ShaderByteCode.size());

	for (size_t i = 0; i < m_ConstantBufferSizes.size(); i++)
	{
		if (m_ConstantBufferSizes[i] > 0)
		{
			m_ConstantBuffers[i] = m_pGraphics->GetOrCreateConstantBuffer((unsigned int)i, (unsigned int)m_ConstantBufferSizes[i]);
		}
		else
		{
			m_ConstantBuffers[i] = nullptr;
		}
	}
	for (auto& pair : m_ShaderParameters)
	{
		pair.second.pBuffer = m_ConstantBuffers[pair.second.Buffer];
	}

	return true;
}
