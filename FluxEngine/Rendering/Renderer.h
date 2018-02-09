#pragma once
#include "Core\Subsystem.h"

class Graphics;
class Drawable;
class Material;
class Camera;
struct Batch;

class Renderer : public Subsystem
{
	FLUX_OBJECT(Renderer, Subsystem)

public:

	Renderer(Context* pContext, Graphics* pGraphics);
	~Renderer();

	void Draw();

	void AddDrawable(Drawable* pDrawable);
	bool RemoveDrawable(Drawable* pDrawable);
	void AddCamera(Camera* pCamera);
	bool RemoveCamera(Camera* pCamera);

	Camera* GetCamera(int camIdx) { return m_Cameras[camIdx]; }

private:
	void SetPerFrameParameters();
	void SetPerCameraParameters(Camera* pCamera);
	void SetPerMaterialParameters(const Material* pMaterial);
	void SetPerBatchParameters(const Batch& batch, Camera* pCamera);

	const Material* m_pCurrentMaterial = nullptr;

	Graphics* m_pGraphics;
	std::vector<Drawable*> m_Drawables;
	std::vector<Camera*> m_Cameras;
	Vector3 m_LightDirection = Vector3(-0.577f, -0.577f, 0.577f);

	int m_CurrentFrame = 0;
	Camera* m_pCurrentCamera = nullptr;
};