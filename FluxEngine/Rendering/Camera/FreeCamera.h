#pragma once
#include "Rendering/Camera/FreeObject.h"

class Camera;
class InputEngine;

class FreeCamera : public FreeObject
{
	FLUX_OBJECT(FreeCamera, SceneNode)

public:
	explicit FreeCamera(Context* pContext);
	virtual ~FreeCamera();

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnSceneRemoved() override;
	Camera* GetCamera() const { return m_pCamera; }

	Camera *m_pCamera = nullptr;
};

