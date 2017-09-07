#include "stdafx.h"
#include "FreeCamera.h"
#include "../Components/Camera.h"
#include "../Components/Transform.h"
#include "../Components/AudioListener.h"

FreeCamera::FreeCamera()
{
}


FreeCamera::~FreeCamera()
{
}

void FreeCamera::Initialize()
{
	m_pCamera = new Camera();
	AddComponent(m_pCamera);
	AudioListener* pAudioListener = new AudioListener();
	AddComponent(pAudioListener);
}

void FreeCamera::Update()
{
	if (m_pCamera->IsActive())
	{
		if(m_UseMouseAndKeyboard)
			KeyboardMouse();
		/*if (m_UseController)
			Controller();*/
	}
}

void FreeCamera::KeyboardMouse()
{
	float dt = GameTimer::DeltaTime();

	//Moving
	XMFLOAT3 moveDirection(0, 0, 0);
	moveDirection.x += InputEngine::GetInstance()->IsKeyboardKeyDown('A') ? -1 : 0;
	moveDirection.x += InputEngine::GetInstance()->IsKeyboardKeyDown('D') ? 1 : 0;
	moveDirection.z += InputEngine::GetInstance()->IsKeyboardKeyDown('S') ? -1 : 0;
	moveDirection.z += InputEngine::GetInstance()->IsKeyboardKeyDown('W') ? 1 : 0;
	moveDirection.y += InputEngine::GetInstance()->IsKeyboardKeyDown('Q') ? -1 : 0;
	moveDirection.y += InputEngine::GetInstance()->IsKeyboardKeyDown('E') ? 1 : 0;

	XMVECTOR xmMove = XMLoadFloat3(&moveDirection);
	
	float moveSpeed = m_MoveSpeed;
	if (InputEngine::GetInstance()->IsKeyboardKeyDown(VK_LSHIFT))
		moveSpeed *= m_ShiftMultiplier;

	xmMove *= dt * moveSpeed;
	GetTransform()->Translate(xmMove, Space::SELF);

	//Rotation
	if (InputEngine::GetInstance()->IsMouseButtonDown(VK_RBUTTON))
	{
		XMFLOAT2 mouseMove = InputEngine::GetInstance()->GetMouseMovement();
		GetTransform()->Rotate(XMFLOAT3(mouseMove.y * dt * m_RotationSpeed, 0.0f, 0.0f), Space::SELF);
		GetTransform()->Rotate(XMFLOAT3(0.0f, mouseMove.x * dt * m_RotationSpeed, 0.0f), Space::WORLD);
	}
}

void FreeCamera::Controller()
{
	float dt = GameTimer::DeltaTime();

	XMFLOAT2 leftStick = InputEngine::GetInstance()->GetThumbstickPosition();
	XMFLOAT2 rightStick = InputEngine::GetInstance()->GetThumbstickPosition(false);
	bool lb = InputEngine::GetInstance()->IsGamepadButtonDown(XINPUT_GAMEPAD_LEFT_SHOULDER);
	bool rb = InputEngine::GetInstance()->IsGamepadButtonDown(XINPUT_GAMEPAD_RIGHT_SHOULDER);
	bool leftStickPress = InputEngine::GetInstance()->IsGamepadButtonDown(XINPUT_GAMEPAD_LEFT_THUMB);

	//Moving
	XMFLOAT3 moveDirection(0, 0, 0);
	moveDirection.x = leftStick.x;
	moveDirection.z = leftStick.y;
	moveDirection.y += lb ? -1 : 0;
	moveDirection.y += rb ? 1 : 0;

	XMVECTOR xmMove = XMLoadFloat3(&moveDirection);

	float moveSpeed = m_MoveSpeed;
	if (leftStickPress)
		moveSpeed *= m_ShiftMultiplier;

	xmMove *= dt * moveSpeed;
	GetTransform()->Translate(xmMove, Space::SELF);

	//Rotation
	GetTransform()->Rotate(XMFLOAT3(-rightStick.y * dt * m_RotationSpeed*m_GamepadSensitivity, 0.0f, 0.0f), Space::SELF);
	GetTransform()->Rotate(XMFLOAT3(0.0f, rightStick.x * dt * m_RotationSpeed*m_GamepadSensitivity, 0.0f), Space::WORLD);
}
