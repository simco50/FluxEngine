#pragma once
#include "Scenegraph\SceneNode.h"

class Camera;
class InputEngine;

class FreeCamera : public SceneNode
{
	FLUX_OBJECT(FreeCamera, SceneNode)

public:
	FreeCamera(Context* pContext);
	virtual ~FreeCamera();

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnSceneRemoved() override;
	void Update();
	Camera* GetCamera() const { return m_pCamera; }

	void UseMouseAndKeyboard(const bool use) { m_UseMouseAndKeyboard = use; }

	void SetSpeed(const float speed) { m_MoveSpeed = speed; }

private:
	InputEngine* m_pInput = nullptr;

	void KeyboardMouse();
	void Controller();

	float m_MoveSpeed = 100.0f;
	float m_ShiftMultiplier = 3.0f;
	float m_RotationSpeed = 20.0f;
	Camera *m_pCamera = nullptr;
	DelegateHandle m_UpdateHandle;

	bool m_UseMouseAndKeyboard = true;
};

