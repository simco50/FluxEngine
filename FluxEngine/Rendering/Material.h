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

	virtual bool Load(const std::string& filePath) override;

	ShaderVariation* GetShader(const ShaderType type) const;
	const std::vector<std::pair<TextureSlot, Texture*>> GetTextures() const { return m_Textures; }
	const std::vector<std::pair<std::string, unsigned int>>& GetShaderParameters() const { return m_Parameters; }
	const std::vector<char>& GetParameterBuffer() const { return m_ParameterBuffer; }

	void SetTexture(const TextureSlot slot, Texture* pTexture);

	void SetCullMode(CullMode mode) { m_CullMode = mode; }
	void SetBlendMode(BlendMode mode) { m_BlendMode = mode; }
	void SetDepthTestMode(CompareMode mode) { m_DepthTestMode = mode; }
	void SetAlphaToCoverage(bool enabled) { m_AlphaToCoverage = enabled; }
	void SetFillMode(FillMode mode) { m_FillMode = mode; }
	void SetDepthEnabled(bool enabled) { m_DepthEnabled = enabled; }

	CullMode GetCullMode() const { return m_CullMode; }
	BlendMode GetBlendMode() const { return m_BlendMode; }
	CompareMode GetDepthTestMode() const { return m_DepthTestMode; }
	bool GetAlphaToCoverage() const { return m_AlphaToCoverage; }
	FillMode GetFillMode() const { return m_FillMode; }
	bool GetDepthEnabled() const { return m_DepthEnabled; }

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
	FillMode m_FillMode = FillMode::SOLID;

	std::vector<std::pair<TextureSlot, Texture*>> m_Textures;

	int m_BufferOffset = 0;
	std::vector<std::pair<std::string, unsigned int>> m_Parameters;
	std::vector<char> m_ParameterBuffer;
};