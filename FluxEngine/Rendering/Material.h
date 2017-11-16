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


private:
	Graphics* m_pGraphics;

	void ParseValue(const std::string name, const std::string valueString);

	string m_Name;
	array<ShaderVariation*, GraphicsConstants::SHADER_TYPES> m_ShaderVariations = {};
	map<string, Shader*> m_Shaders;

	CullMode m_CullMode = CullMode::BACK;
	bool m_Blending = false;

	vector<pair<TextureSlot, Texture*>> m_Textures;
	vector<unique_ptr<Texture>> m_TextureCache;

	int m_BufferOffset = 0;
	vector<pair<string, unsigned int>> m_Parameters;
	vector<char> m_ParameterBuffer;
};