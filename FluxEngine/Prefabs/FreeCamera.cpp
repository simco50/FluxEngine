#include "stdafx.h"
#include "FreeCamera.h"
#include "../Components/CameraComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/AudioListener.h"

FreeCamera::FreeCamera()
{
}


FreeCamera::~FreeCamera()
{
}

void FreeCamera::Initialize()
{
	m_pCamera = new CameraComponent();
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
		if (m_UseController)
			Controller();
	}
}

void FreeCamera::KeyboardMouse()
{
	float dt = m_pGameContext->Scene->GameTimer.DeltaTime();

	//Moving
	XMFLOAT3 moveDirection(0, 0, 0);
	moveDirection.x += m_pGameContext->Scene->Input->IsKeyboardKeyDown('A') ? -1 : 0;
	moveDirection.x += m_pGameContext->Scene->Input->IsKeyboardKeyDown('D') ? 1 : 0;
	moveDirection.z += m_pGameContext->Scene->Input->IsKeyboardKeyDown('S') ? -1 : 0;
	moveDirection.z += m_pGameContext->Scene->Input->IsKeyboardKeyDown('W') ? 1 : 0;
	moveDirection.y += m_pGameContext->Scene->Input->IsKeyboardKeyDown('Q') ? -1 : 0;
	moveDirection.y += m_pGameContext->Scene->Input->IsKeyboardKeyDown('E') ? 1 : 0;

	XMVECTOR xmMove = XMLoadFloat3(&moveDirection);

	float moveSpeed = m_MoveSpeed;
	if (m_pGameContext->Scene->Input->IsKeyboardKeyDown(VK_LSHIFT))
		moveSpeed *= m_ShiftMultiplier;

	xmMove *= dt * moveSpeed;
	GetTransform()->Translate(xmMove, Space::SELF);

	//Rotation
	if (m_pGameContext->Scene->Input->IsMouseButtonDown(VK_RBUTTON))
	{
		XMFLOAT2 mouseMove = m_pGameContext->Scene->Input->GetMouseMovement();
		GetTransform()->Rotate(XMFLOAT3(mouseMove.y * dt * m_RotationSpeed, 0.0f, 0.0f), Space::SELF);
		GetTransform()->Rotate(XMFLOAT3(0.0f, mouseMove.x * dt * m_RotationSpeed, 0.0f), Space::WORLD);
	}
}

void FreeCamera::Controller()
{
	float dt = m_pGameContext->Scene->GameTimer.DeltaTime();

	XMFLOAT2 leftStick = m_pGameContext->Scene->Input->GetThumbstickPosition();
	XMFLOAT2 rightStick = m_pGameContext->Scene->Input->GetThumbstickPosition(false);
	bool lb = m_pGameContext->Scene->Input->IsGamepadButtonDown(XINPUT_GAMEPAD_LEFT_SHOULDER);
	bool rb = m_pGameContext->Scene->Input->IsGamepadButtonDown(XINPUT_GAMEPAD_RIGHT_SHOULDER);
	bool leftStickPress = m_pGameContext->Scene->Input->IsGamepadButtonDown(XINPUT_GAMEPAD_LEFT_THUMB);

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
