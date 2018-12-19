#pragma once
#include "Rendering\Core\GraphicsDefines.h"
#include "Content\Resource.h"

class Shader;
class ShaderVariation;
class Graphics;
class Texture;

namespace tinyxml2
{
	class XMLElement;
}

class Material : public Resource
{
	FLUX_OBJECT(Material, Resource)

public:
	Material(Context* pContext);
	~Material();

	class ParameterEntry
	{
	public:
		ParameterEntry() :
			m_Size(0),
			m_pDataPool(nullptr),
			m_DataOffset(0)
		{}
		ParameterEntry(size_t size, void* pInData, std::vector<char>& dataPool) :
			m_Size(size), m_pDataPool(&dataPool)
		{
			m_DataOffset = (int)dataPool.size();
			dataPool.resize(dataPool.size() + size);
			if (pInData)
			{
				memcpy(dataPool.data() + m_DataOffset, pInData, size);
			}
		}
		void SetData(const void* pInData, size_t size)
		{
			assert(size == m_Size);
			memcpy(m_pDataPool->data() + m_DataOffset, pInData, size);
		}
		void* GetData() const { return m_pDataPool->data() + m_DataOffset; }
		size_t GetSize() const { return m_Size; }
	private:
		size_t m_Size;
		std::vector<char>* m_pDataPool;
		int m_DataOffset;
	};

private:
	class ParameterCache
	{
	public:
		ParameterEntry& GetParameter(const std::string& name, const size_t size)
		{
			ParameterEntry& entry = m_Parameters[StringHash(name)];
			if (entry.GetSize() != size)
			{
				entry = ParameterEntry(size, nullptr, m_ParameterPool);
			}
			return entry;
		}
		const std::unordered_map<StringHash, ParameterEntry>& GetParameters() const { return m_Parameters; }

		size_t ByteSize() const
		{
			return m_Parameters.size() * sizeof(ParameterEntry) + m_ParameterPool.size();
		}

	private:
		std::vector<char> m_ParameterPool;
		std::unordered_map<StringHash, ParameterEntry> m_Parameters;
	};

public:
	virtual bool Load(InputStream& inputStream) override;

	ShaderVariation* GetShader(ShaderType type) const;
	const std::unordered_map<TextureSlot, Texture*>& GetTextures() const { return m_Textures; }
	const std::unordered_map<StringHash, ParameterEntry>& GetShaderParameters() const { return m_ParameterCache.GetParameters(); }

	void SetTexture(TextureSlot slot, Texture* pTexture);
	void SetShader(ShaderType type, ShaderVariation* pShader);

	void SetCullMode(CullMode mode) { m_CullMode = mode; }
	void SetBlendMode(BlendMode mode) { m_BlendMode = mode; }
	void SetDepthTestMode(CompareMode mode) { m_DepthTestMode = mode; }
	void SetAlphaToCoverage(bool enabled) { m_AlphaToCoverage = enabled; }
	void SetFillMode(FillMode mode) { m_FillMode = mode; }
	void SetDepthEnabled(bool enabled) { m_DepthEnabled = enabled; }
	void SetDepthWrite(bool enabled) { m_DepthWrite = enabled; }

	const std::string& GetName() const { return m_Name; }

	CullMode GetCullMode() const { return m_CullMode; }
	BlendMode GetBlendMode() const { return m_BlendMode; }
	CompareMode GetDepthTestMode() const { return m_DepthTestMode; }
	bool GetAlphaToCoverage() const { return m_AlphaToCoverage; }
	FillMode GetFillMode() const { return m_FillMode; }
	bool GetDepthEnabled() const { return m_DepthEnabled; }
	bool GetDepthWrite() const { return m_DepthWrite; }

private:
	Graphics* m_pGraphics;
	bool ParseShaders(tinyxml2::XMLElement* pElement);
	bool ParseProperties(tinyxml2::XMLElement* pElement);
	bool ParseParameters(tinyxml2::XMLElement* pElement);

	void ParseValue(const std::string& name, const std::string& valueString);

	void RefreshMemoryUsage();

	std::string m_Name;
	std::array<ShaderVariation*, (size_t)ShaderType::MAX> m_ShaderVariations = {};

	//Properties
	CullMode m_CullMode = CullMode::BACK;
	BlendMode m_BlendMode = BlendMode::REPLACE;
	CompareMode m_DepthTestMode = CompareMode::LESSEQUAL;
	bool m_AlphaToCoverage = false;
	bool m_DepthEnabled = true;
	bool m_DepthWrite = true;
	FillMode m_FillMode = FillMode::SOLID;

	std::string m_InternalDefines;
	std::unordered_map<TextureSlot, Texture*> m_Textures;
	ParameterCache m_ParameterCache;
};