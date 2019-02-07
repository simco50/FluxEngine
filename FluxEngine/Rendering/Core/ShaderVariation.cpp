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
	outputStream.WriteInt32(SHADER_CACHE_VERSION);
	outputStream.WriteSizedString(m_Name);
	outputStream.WriteInt32(m_ShaderModel.MajVersion);
	outputStream.WriteInt32(m_ShaderModel.MinVersion);
	outputStream.WriteUByte((unsigned char)m_ShaderType);
	outputStream.WriteUByte((unsigned char)m_Defines.size());
	for (const std::string& define : m_Defines)
	{
		outputStream.WriteSizedString(define);
	}
	outputStream.WriteUByte((unsigned char)m_ShaderParameters.size());
	for (const auto& pair : m_ShaderParameters)
	{
		outputStream.WriteUint32(pair.first.m_Hash);
		const ShaderParameter& parameter = pair.second;
		outputStream.WriteSizedString(parameter.Name);
		outputStream.WriteInt32(parameter.Buffer);
		outputStream.WriteInt32(parameter.Size);
		outputStream.WriteInt32(parameter.Offset);
	}
	for (size_t size : m_ConstantBufferSizes)
	{
		outputStream.WriteInt32((int)size);
	}
	outputStream.WriteInt32((int)m_ShaderByteCode.size());
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
	int version = inputStream.ReadInt32();
	if (version != SHADER_CACHE_VERSION)
	{
		FLUX_LOG(Warning, "[ShaderVariation::LoadFromCache()] > Cached shader version mismatch: %i, expected %i", version, SHADER_CACHE_VERSION);
		return false;
	}
	m_Name = inputStream.ReadSizedString();
	m_ShaderModel.MajVersion = inputStream.ReadInt32();
	m_ShaderModel.MinVersion = inputStream.ReadInt32();
	ShaderModel desired = GetDesiredShaderModel();
	if (desired.MajVersion != m_ShaderModel.MajVersion || desired.MinVersion != m_ShaderModel.MinVersion)
	{
		return false;
	}
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
		StringHash parameterHash = StringHash(inputStream.ReadUint32());
		ShaderParameter& parameter = m_ShaderParameters[parameterHash];

		parameter.Name = inputStream.ReadSizedString();
		parameter.Buffer = inputStream.ReadInt32();
		parameter.Size = inputStream.ReadInt32();
		parameter.Offset = inputStream.ReadInt32();
	}
	for (size_t& size : m_ConstantBufferSizes)
	{
		size = (size_t)inputStream.ReadInt32();
	}

	m_ShaderByteCode.resize((size_t)inputStream.ReadInt32());
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
