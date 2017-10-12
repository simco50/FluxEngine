#pragma once
#include "Material.h"

enum class ParticleBlendMode;
class Texture;

class ParticleMaterial :
	public Material
{
public:
	ParticleMaterial();
	~ParticleMaterial();
	virtual ID3DX11EffectTechnique* GetTechnique() const override;
	void SetBlendMode(const ParticleBlendMode mode);
	void SetTexture(const string& path);

protected:
	virtual void LoadShaderVariables() override;
	virtual void UpdateShaderVariables() override;

private:

	Texture* m_pTexture = nullptr;
	ParticleBlendMode m_BlendMode;

	ID3DX11EffectTechnique *m_pAlphaBlendingTechnique = nullptr;
	ID3DX11EffectTechnique *m_pAdditiveBlendingTechnique = nullptr;

	ID3DX11EffectMatrixVariable* m_pViewProjVariable = nullptr;
	ID3DX11EffectShaderResourceVariable* m_pTextureVariable = nullptr;
};

