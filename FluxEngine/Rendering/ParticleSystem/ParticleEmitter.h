#pragma once
#include "Rendering/Drawable.h"

class Particle;
class Texture;
class Graphics;
class VertexBuffer;
class ParticleSystem;
class Material;

class ParticleEmitter : public Drawable
{
public:
	ParticleEmitter(Graphics* pGraphics, ParticleSystem* pSystem);
	~ParticleEmitter();
	ParticleEmitter(const ParticleEmitter& other) = delete;
	ParticleEmitter operator=(const ParticleEmitter& other) = delete;

	ParticleSystem* GetSettings() const { return m_pParticleSystem; }

	void SetSystem(ParticleSystem* pSettings);

	void Reset();

	void Play() { m_Playing = true; }
	void Stop() { m_Playing = false; }

	int GetParticleCount() const { return m_ParticleCount; }

protected:
	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnNodeSet(SceneNode* pNode) override;
	virtual void Update() override;

private:
	void CreateVertexBuffer();
	void SortParticles();

	void CalculateBoundingBox();

	bool m_Playing = false;
	float m_Timer = 0.0f;

	map<float, int>::iterator m_BurstIterator;

	vector<Particle*> m_Particles;
	ParticleSystem* m_pParticleSystem = nullptr;

	int m_ParticleCount = 0;
	int m_BufferSize = 0;
	float m_ParticleSpawnTimer = 0.0f;

	unique_ptr<Texture> m_pTexture;
	unique_ptr<Geometry> m_pGeometry;
	unique_ptr<VertexBuffer> m_pVertexBuffer;
	unique_ptr<Material> m_pMaterial;

	wstring m_AssetFile;

	Graphics* m_pGraphics;
};

