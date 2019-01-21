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
	PostProcessing(Context* pContext);
	virtual ~PostProcessing();

	void Draw();
	void AddEffect(Material* pMaterial, const bool active = true);
	uint32 GetMaterialCount() const { return (uint32)m_Materials.size(); }
	Material* GetMaterial(const uint32 index) const { return m_Materials[index].second; }
	void SetMaterialActive(const uint32 index, const bool active) { m_Materials[index].first = active; }
	bool& GetMaterialActive(const uint32 index) { return m_Materials[index].first; }
	Camera* GetCamera() const { return m_pCamera; }

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