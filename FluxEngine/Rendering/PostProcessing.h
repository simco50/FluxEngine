#pragma once
#include "Scenegraph/Component.h"

class Graphics;
class ShaderVariation;
class Material;
class Texture2D;
class Camera;
class Renderer;

class PostProcessing : public Component
{
	FLUX_OBJECT(PostProcessing, Component)

public:
	explicit PostProcessing(Context* pContext);
	virtual ~PostProcessing();

	void Draw();
	void AddEffect(Material* pMaterial, const bool active = true);
	Camera* GetCamera() const { return m_pCamera; }

	virtual void CreateUI() override;

protected:
	virtual void OnNodeSet(SceneNode* pNode) override;

private:
	void OnResize(const int width, const int height);

	std::unique_ptr<Texture2D> m_pRenderTexture;
	std::unique_ptr<Texture2D> m_pDepthTexture;

	Camera* m_pCamera = nullptr;
	Graphics* m_pGraphics = nullptr;
	Renderer* m_pRenderer = nullptr;

	ShaderVariation* m_pBlitVertexShader;
	ShaderVariation* m_pBlitPixelShader;
	std::vector<std::pair<bool, Material*>> m_Materials;
};