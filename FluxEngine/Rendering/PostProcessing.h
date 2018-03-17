#pragma once

class RenderTarget;
class Graphics;
class ShaderVariation;
class Material;

#include "Core/Subsystem.h"

class PostProcessing : public Subsystem
{
	FLUX_OBJECT(PostProcessing, Subsystem)

public:
	PostProcessing(Context* pContext);
	virtual ~PostProcessing();

	void Draw();

	void AddEffect(Material* pMaterial);

private:
	void OnResize(const int width, const int height);
	std::unique_ptr<RenderTarget> m_pIntermediateRenderTarget;
	Graphics* m_pGraphics;

	ShaderVariation* m_pBlitVertexShader;
	ShaderVariation* m_pBlitPixelShader;
	std::vector<Material*> m_Materials;
};