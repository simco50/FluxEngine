#pragma once
#include "Rendering\Core\GraphicsDefines.h"

class Shader;
class ShaderVariation;
class Graphics;
class Texture;

class Material
{
public:
	Material(Graphics* pGraphics)
	{
		UNREFERENCED_PARAMETER(pGraphics);
	}
	~Material();

	static unique_ptr<Material> Load(std::string filePath, Graphics* pGraphics);

	ShaderVariation* GetShader(const ShaderType type) const { return m_ShaderVariations[(unsigned int)type]; }
	Texture* GetTexture(const TextureSlot slot) const { return m_Textures.at(slot); }

private:
	string m_Name;
	array<ShaderVariation*, GraphicsConstants::SHADER_TYPES> m_ShaderVariations = {};
	map<string, Shader*> m_Shaders;

	CullMode m_CullMode = CullMode::BACK;
	bool m_Blending = false;

	map<TextureSlot, Texture*> m_Textures;
};