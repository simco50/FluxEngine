#pragma once
#include "Rendering\Core\GraphicsDefines.h"
#include "Content\Resource.h"

class Shader;
class ShaderVariation;
class Graphics;
class Texture;

class Material : public Resource
{
public:
	Material(Graphics* pGraphics) :
		m_pGraphics(pGraphics)
	{
	}
	~Material();

	virtual bool Load(const std::string& filePath) override;

	ShaderVariation* GetShader(const ShaderType type) const;
	const vector<pair<TextureSlot, Texture*>> GetTextures() const { return m_Textures; }
	const vector<pair<string, unsigned int>>& GetShaderParameters() const { return m_Parameters; }
	const vector<char>& GetParameterBuffer() const { return m_ParameterBuffer; }

	void SetCullMode(CullMode mode) { m_CullMode = mode; }
	void SetBlendMode(BlendMode mode) { m_BlendMode = mode; }
	void SetDepthTestMode(CompareMode mode) { m_DepthTestMode = mode; }
	void SetAlphaToCoverage(bool enabled) { m_AlphaToCoverage = enabled; }
	void SetFillMode(FillMode mode) { m_FillMode = mode; }

	CullMode GetCullMode() const { return m_CullMode; }
	BlendMode GetBlendMode() const { return m_BlendMode; }
	CompareMode GetDepthTestMode() const { return m_DepthTestMode; }
	bool GetAlphaToCoverage() const { return m_AlphaToCoverage; }
	FillMode GetFillMode() const { return m_FillMode; }

private:
	Graphics* m_pGraphics;

	void ParseValue(const std::string name, const std::string valueString);

	string m_Name;
	array<ShaderVariation*, GraphicsConstants::SHADER_TYPES> m_ShaderVariations = {};
	map<string, Shader*> m_Shaders;
	
	//Properties
	CullMode m_CullMode = CullMode::BACK;
	BlendMode m_BlendMode = BlendMode::REPLACE;
	CompareMode m_DepthTestMode = CompareMode::LESSEQUAL;
	bool m_AlphaToCoverage = false;
	FillMode m_FillMode = FillMode::SOLID;

	vector<pair<TextureSlot, Texture*>> m_Textures;
	vector<unique_ptr<Texture>> m_TextureCache;

	int m_BufferOffset = 0;
	vector<pair<string, unsigned int>> m_Parameters;
	vector<char> m_ParameterBuffer;
};