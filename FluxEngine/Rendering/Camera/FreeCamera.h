#pragma once
#include "Scenegraph\SceneNode.h"

class FreeCamera : public SceneNode
{
public:
	FreeCamera(InputEngine* pInput, Graphics* pGraphics);
	virtual ~FreeCamera();

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void Update() override;
	Camera* GetCamera() const { return m_pCamera; }

	void UseMouseAndKeyboard(const bool use) { m_UseMouseAndKeyboard = use; }
	void UseController(const bool use) { m_UseController = use; }

	void SetSpeed(const float speed) { m_MoveSpeed = speed; }

private:
	InputEngine* m_pInput = nullptr;
	Graphics* m_pGraphics = nullptr;

	void KeyboardMouse();
	void Controller();

	float m_MoveSpeed = 5.0f;
	float m_ShiftMultiplier = 3.0f;
	float m_RotationSpeed = 20.0f;
	Camera *m_pCamera = nullptr;

	bool m_UseMouseAndKeyboard = true;
	bool m_UseController = true;
	float m_GamepadSensitivity = 4.0f;
};

