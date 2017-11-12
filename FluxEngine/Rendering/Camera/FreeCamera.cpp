#include "stdafx.h"
#include "FreeCamera.h"
#include "Camera.h"
#include "Core/Components/Transform.h"
#include "Audio/AudioListener.h"

FreeCamera::FreeCamera(InputEngine* pInput, Graphics* pGraphics) : 
	m_pInput(pInput), m_pGraphics(pGraphics)
{
}


FreeCamera::~FreeCamera()
{
}

void FreeCamera::OnSceneSet(Scene* pScene)
{
	SceneNode::OnSceneSet(pScene);

	m_pCamera = new Camera(m_pInput, m_pGraphics);
	AddComponent(m_pCamera);
	AudioListener* pAudioListener = new AudioListener();
	AddComponent(pAudioListener);
}

void FreeCamera::Update()
{
	SceneNode::Update();

	if(m_UseMouseAndKeyboard)
		KeyboardMouse();
	if (m_UseController)
		Controller();
}

void FreeCamera::KeyboardMouse()
{
	float dt = GameTimer::DeltaTime();

	//Moving
	Vector3 moveDirection(0, 0, 0);
	moveDirection.x += m_pInput->IsKeyboardKeyDown('A') ? -1 : 0;
	moveDirection.x += m_pInput->IsKeyboardKeyDown('D') ? 1 : 0;
	moveDirection.z += m_pInput->IsKeyboardKeyDown('S') ? -1 : 0;
	moveDirection.z += m_pInput->IsKeyboardKeyDown('W') ? 1 : 0;
	moveDirection.y += m_pInput->IsKeyboardKeyDown('Q') ? -1 : 0;
	moveDirection.y += m_pInput->IsKeyboardKeyDown('E') ? 1 : 0;

	float moveSpeed = m_MoveSpeed;
	if (m_pInput->IsKeyboardKeyDown(VK_LSHIFT))
		moveSpeed *= m_ShiftMultiplier;
	moveDirection *= dt * moveSpeed;

	GetTransform()->Translate(moveDirection, Space::SELF);

	//Rotation
	if ( m_pInput->IsMouseButtonDown(VK_RBUTTON))
	{
		Vector2 mouseMove =  m_pInput->GetMouseMovement();
		GetTransform()->Rotate(mouseMove.y * dt * m_RotationSpeed, 0.0f, 0.0f, Space::SELF);
		GetTransform()->Rotate(0.0f, mouseMove.x * dt * m_RotationSpeed, 0.0f, Space::WORLD);
	}
}

void FreeCamera::Controller()
{
	float dt = GameTimer::DeltaTime();

	Vector2 leftStick =  m_pInput->GetThumbstickPosition();
	Vector2 rightStick =  m_pInput->GetThumbstickPosition(false);
	bool lb =  m_pInput->IsGamepadButtonDown(XINPUT_GAMEPAD_LEFT_SHOULDER);
	bool rb =  m_pInput->IsGamepadButtonDown(XINPUT_GAMEPAD_RIGHT_SHOULDER);
	bool leftStickPress =  m_pInput->IsGamepadButtonDown(XINPUT_GAMEPAD_LEFT_THUMB);

	//Moving
	Vector3 moveDirection(0, 0, 0);
	moveDirection.x = leftStick.x;
	moveDirection.z = leftStick.y;
	moveDirection.y += lb ? -1 : 0;
	moveDirection.y += rb ? 1 : 0;

	float moveSpeed = m_MoveSpeed;
	if (leftStickPress)
		moveSpeed *= m_ShiftMultiplier;

	moveDirection *= dt * moveSpeed;

	GetTransform()->Translate(moveDirection, Space::SELF);

	//Rotation
	GetTransform()->Rotate(XMFLOAT3(-rightStick.y * dt * m_RotationSpeed*m_GamepadSensitivity, 0.0f, 0.0f), Space::SELF);
	GetTransform()->Rotate(XMFLOAT3(0.0f, rightStick.x * dt * m_RotationSpeed*m_GamepadSensitivity, 0.0f), Space::WORLD);
}