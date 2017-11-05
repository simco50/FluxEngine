#pragma once
#include "Material.h"

enum class ParticleBlendMode;
class Texture;

class ParticleMaterial : public Material
{
public:
	ParticleMaterial(Graphics* pGraphics);
	~ParticleMaterial();
	void SetBlendMode(const ParticleBlendMode mode);
	void SetTexture(const string& path);

protected:

private:
	Texture* m_pTexture = nullptr;
	ParticleBlendMode m_BlendMode;
};

