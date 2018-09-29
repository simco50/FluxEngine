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
class StructuredBuffer;
struct Batch;

DECLARE_MULTICAST_DELEGATE(OnSceneUpdateDelegate);

class Renderer : public Subsystem
{
	FLUX_OBJECT(Renderer, Subsystem)

public:

	Renderer(Context* pContext);
	~Renderer();

	void Draw();

	void AddDrawable(Drawable* pDrawable);
	void RemoveDrawable(Drawable* pDrawable);
	void AddCamera(Camera* pCamera);
	void RemoveCamera(Camera* pCamera);
	void AddLight(Light* pLight);
	void RemoveLight(Light* pLight);

	Camera* GetCamera(int camIdx) { return m_Cameras[camIdx]; }

	Geometry* GetQuadGeometry() const { return m_pQuadGeometry.get(); }

	void QueueCamera(Camera* pCamera);

	OnSceneUpdateDelegate& OnPreRender() { return m_OnPreRender; }

private:
	void CreateQuadGeometry();

	void SetPerFrameParameters();
	void SetPerCameraParameters(Camera* pCamera);
	void SetPerMaterialParameters(const Material* pMaterial);
	void SetPerBatchParameters(const Batch& batch, Camera* pCamera);

	const Material* m_pCurrentMaterial = nullptr;

	OnSceneUpdateDelegate m_OnPreRender;

	Graphics* m_pGraphics;
	std::vector<Drawable*> m_Drawables;
	std::vector<Camera*> m_Cameras;
	std::vector<Light*> m_Lights;

	int m_CurrentFrame = 0;
	Camera* m_pCurrentCamera = nullptr;

	std::vector<Camera*> m_CameraQueue;

	std::unique_ptr<VertexBuffer> m_pQuadVertexBuffer;
	std::unique_ptr<IndexBuffer> m_pQuadIndexBuffer;
	std::unique_ptr<Geometry> m_pQuadGeometry;
	std::unique_ptr<StructuredBuffer> m_pLightBuffer;
};