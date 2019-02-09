#pragma once
#include "Core\Subsystem.h"

class Graphics;
class Drawable;
class Material;
class Camera;
class IndexBuffer;
class VertexBuffer;
class Geometry;
class Light;
class RenderTarget;
class PostProcessing;
class GraphicsCommandContext;
struct View;
struct Batch;

DECLARE_MULTICAST_DELEGATE(OnSceneUpdateDelegate);

class Renderer : public Subsystem
{
	FLUX_OBJECT(Renderer, Subsystem)

public:
	explicit Renderer(Context* pContext);
	~Renderer();

	void Draw();

	void AddDrawable(Drawable* pDrawable);
	void RemoveDrawable(Drawable* pDrawable);
	void AddCamera(Camera* pCamera);
	void RemoveCamera(Camera* pCamera);
	void AddLight(Light* pLight);
	void RemoveLight(Light* pLight);
	void AddPostProcessing(PostProcessing* pPostProcessing);
	void RemovePostProcessing(PostProcessing* pPostProcessing);

	void Blit(GraphicsCommandContext* pCommandContext, RenderTarget* pSource, RenderTarget* pTarget, Material* pMaterial = nullptr);

	Camera* GetCamera(int camIdx) { return m_Cameras[camIdx]; }

	Geometry* GetQuadGeometry() const { return m_pQuadGeometry.get(); }

	void QueueView(const View* pView);

	OnSceneUpdateDelegate& OnPreRender() { return m_OnPreRender; }

private:
	void CreateQuadGeometry();

	void SetPerFrameParameters(GraphicsCommandContext* pCommandContext);
	void SetPerViewParameters(GraphicsCommandContext* pCommandContext, const View* pView);
	void SetPerMaterialParameters(GraphicsCommandContext* pCommandContext, const Material* pMaterial);
	void SetPerBatchParameters(GraphicsCommandContext* pCommandContext, const Batch& batch, const View* pView);

	const Material* m_pCurrentMaterial = nullptr;

	OnSceneUpdateDelegate m_OnPreRender;

	Graphics* m_pGraphics;
	std::vector<Drawable*> m_Drawables;
	std::vector<Camera*> m_Cameras;
	std::vector<Light*> m_Lights;
	std::vector<PostProcessing*> m_PostProcessing;

	Material* m_pBlitMaterial = nullptr;

	std::vector<const View*> m_ViewQueue;

	std::unique_ptr<VertexBuffer> m_pQuadVertexBuffer;
	std::unique_ptr<IndexBuffer> m_pQuadIndexBuffer;
	std::unique_ptr<Geometry> m_pQuadGeometry;
};