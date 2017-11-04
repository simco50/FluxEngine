#pragma once
#include "Rendering\Core\GraphicsDefines.h"
class Shader;
class ShaderVariation;

enum class RenderQueueID
{
	ID_OPAQUE,
	ID_TRANSPARANT,
	ID_DEPTH,
};

struct MaterialDesc
{
	string ShaderFilePath = "";
	RenderQueueID RenderQueue = RenderQueueID::ID_OPAQUE;
	ShaderType ShaderMask = ShaderType::NONE;

	bool Validate() const
	{
		if (ShaderFilePath.size() == 0)
		{
			FLUX_LOG(ERROR, "Material::Initialize() > MaterialDesc is invalid!");
			return false;
		}
		return true;
	}
};

class Material
{
public:
	Material(Graphics* pGraphics);
	virtual ~Material();

	void LoadShader();
	void Update();

	const MaterialDesc& GetDesc() const { return m_MaterialDesc; }
	array<ShaderVariation*, GraphicsConstants::SHADER_TYPES> GetShaders() const {	return m_ShaderVariations; }

protected:
	virtual void UpdateShaderVariables() = 0;

	MaterialDesc m_MaterialDesc;
	unique_ptr<Shader> m_pShader;
	array<ShaderVariation*, GraphicsConstants::SHADER_TYPES> m_ShaderVariations = {};
	Graphics* m_pGraphics;

private:
};

