#pragma once
#include "Scenegraph\SceneNode.h"

class Graphics;
class Camera;
class InputEngine;

class FreeCamera : public SceneNode
{
	FLUX_OBJECT(FreeCamera, SceneNode)

public:
	FreeCamera(Context* pContext);
	virtual ~FreeCamera();

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void Update() override;
	Camera* GetCamera() const { return m_pCamera; }

	void UseMouseAndKeyboard(const bool use) { m_UseMouseAndKeyboard = use; }

	void SetSpeed(const float speed) { m_MoveSpeed = speed; }

private:
	InputEngine* m_pInput = nullptr;
	Graphics* m_pGraphics = nullptr;

	void KeyboardMouse();

	float m_MoveSpeed = 50.0f;
	float m_ShiftMultiplier = 3.0f;
	float m_RotationSpeed = 20.0f;
	Camera *m_pCamera = nullptr;

	bool m_UseMouseAndKeyboard = true;
};

