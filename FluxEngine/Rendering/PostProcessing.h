#pragma once

class Graphics;
class ShaderVariation;
class Material;
class Texture2D;

#include "Core/Subsystem.h"

class PostProcessing : public Subsystem
{
	FLUX_OBJECT(PostProcessing, Subsystem)

public:
	PostProcessing(Context* pContext);
	virtual ~PostProcessing();

	void Draw();
	void AddEffect(Material* pMaterial, const bool active = true);
	uint32 GetMaterialCount() const { return (uint32)m_Materials.size(); }
	Material* GetMaterial(const uint32 index) const { return m_Materials[index].second; }
	void SetMaterialActive(const uint32 index, const bool active) { m_Materials[index].first = active; }
	bool& GetMaterialActive(const uint32 index) { return m_Materials[index].first; }
	
private:
	void OnResize(const int width, const int height);

	std::unique_ptr<Texture2D> m_pRenderTexture;
	std::unique_ptr<Texture2D> m_pDepthTexture;

	Graphics* m_pGraphics;

	ShaderVariation* m_pBlitVertexShader;
	ShaderVariation* m_pBlitPixelShader;
	std::vector<std::pair<bool, Material*>> m_Materials;
};