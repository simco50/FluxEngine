#pragma once
#include "..\Scenegraph\GameObject.h"

class FreeCamera : public GameObject
{
public:
	FreeCamera();
	~FreeCamera();

	void Initialize();
	void Update();
	Camera* GetCamera() const { return m_pCamera; }

	void UseMouseAndKeyboard(const bool use) { m_UseMouseAndKeyboard = use; }
	void UseController(const bool use) { m_UseController = use; }

	void SetSpeed(const float speed) { m_MoveSpeed = speed; }

private:
	void KeyboardMouse();
	void Controller();

	float m_MoveSpeed = 10.0f;
	float m_ShiftMultiplier = 3.0f;
	float m_RotationSpeed = 30.0f;
	Camera *m_pCamera = nullptr;

	bool m_UseMouseAndKeyboard = true;
	bool m_UseController = true;
	float m_GamepadSensitivity = 4.0f;
};

