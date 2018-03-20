#pragma once
#include "Rendering\Core\GraphicsDefines.h"
#include "Content\Resource.h"

class Shader;
class ShaderVariation;
class Graphics;
class Texture;

class Material : public Resource
{
	FLUX_OBJECT(Material, Resource)

public:
	Material(Context* pContext);
	~Material();

	struct ParameterEntry
	{
		ParameterEntry()
		{}
		ParameterEntry(const size_t size, void* pData) :
			Size(size), pData(pData)
		{}
		size_t Size;
		void* pData;
	};

	virtual bool Load(InputStream& inputStream) override;

	ShaderVariation* GetShader(const ShaderType type) const;
	const std::unordered_map<TextureSlot, Texture*> GetTextures() const { return m_Textures; }
	const std::unordered_map<std::string, ParameterEntry>& GetShaderParameters() const { return m_Parameters; }

	void SetTexture(const TextureSlot slot, Texture* pTexture);

	void SetCullMode(CullMode mode) { m_CullMode = mode; }
	void SetBlendMode(BlendMode mode) { m_BlendMode = mode; }
	void SetDepthTestMode(CompareMode mode) { m_DepthTestMode = mode; }
	void SetAlphaToCoverage(bool enabled) { m_AlphaToCoverage = enabled; }
	void SetFillMode(FillMode mode) { m_FillMode = mode; }
	void SetDepthEnabled(bool enabled) { m_DepthEnabled = enabled; }
	void SetDepthWrite(bool enabled) { m_DepthWrite = enabled; }


	CullMode GetCullMode() const { return m_CullMode; }
	BlendMode GetBlendMode() const { return m_BlendMode; }
	CompareMode GetDepthTestMode() const { return m_DepthTestMode; }
	bool GetAlphaToCoverage() const { return m_AlphaToCoverage; }
	FillMode GetFillMode() const { return m_FillMode; }
	bool GetDepthEnabled() const { return m_DepthEnabled; }
	bool GetDepthWrite() const { return m_DepthWrite; }

private:
	Graphics* m_pGraphics;

	void ParseValue(const std::string& name, const std::string& valueString);

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

	std::unordered_map<TextureSlot, Texture*> m_Textures;

	std::unordered_map<std::string, ParameterEntry> m_Parameters;
	int m_BufferOffset = 0;
};