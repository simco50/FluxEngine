#include "FluxEngine.h"
#include "FreeCamera.h"
#include "Camera.h"
#include "Audio/AudioListener.h"
#include "Input/InputEngine.h"
#include "Scenegraph/Scene.h"

FreeCamera::FreeCamera(Context* pContext) :
	SceneNode(pContext)
{
	m_pInput = pContext->GetSubsystem<InputEngine>();
}

FreeCamera::~FreeCamera()
{
}

void FreeCamera::OnSceneSet(Scene* pScene)
{
	SceneNode::OnSceneSet(pScene);

	if (GetSubsystem<AudioEngine>(false))
	{
		CreateComponent<AudioListener>();
	}
	m_pCamera = CreateComponent<Camera>();

	m_UpdateHandle = pScene->OnSceneUpdate().AddRaw(this, &FreeCamera::Update);
}

void FreeCamera::OnSceneRemoved()
{
	m_pScene->OnSceneUpdate().Remove(m_UpdateHandle);
	SceneNode::OnSceneRemoved();
}

void FreeCamera::Update()
{
	if (m_pCamera->IsEnabled())
	{
		if (m_UseMouseAndKeyboard)
		{
			KeyboardMouse();
		}
		else
		{
			Controller();
		}
	}
}

void FreeCamera::KeyboardMouse()
{
	float dt = GameTimer::DeltaTime();

	//Moving
	Vector3 moveDirection(0, 0, 0);
	moveDirection.x += m_pInput->IsKeyboardKeyDown(KeyboardKey::KEY_A) ? -1 : 0;
	moveDirection.x += m_pInput->IsKeyboardKeyDown(KeyboardKey::KEY_D) ? 1 : 0;
	moveDirection.z += m_pInput->IsKeyboardKeyDown(KeyboardKey::KEY_S) ? -1 : 0;
	moveDirection.z += m_pInput->IsKeyboardKeyDown(KeyboardKey::KEY_W) ? 1 : 0;
	moveDirection.y += m_pInput->IsKeyboardKeyDown(KeyboardKey::KEY_Q) ? -1 : 0;
	moveDirection.y += m_pInput->IsKeyboardKeyDown(KeyboardKey::KEY_E) ? 1 : 0;

	float moveSpeed = m_MoveSpeed;
	if (m_pInput->IsKeyboardKeyDown(KeyboardKey::KEY_LEFT_SHIFT))
	{
		moveSpeed *= m_ShiftMultiplier;
	}
	moveDirection *= dt * moveSpeed;

	if (moveDirection != Vector3())
	{
		Translate(moveDirection, Space::Self);
	}

	//Rotation
	if ( m_pInput->IsMouseButtonDown(MouseKey::RIGHT_BUTTON))
	{
		Vector2 mouseMove =  m_pInput->GetMouseMovement();
		if (mouseMove != Vector2())
		{
			Rotate(mouseMove.y * dt * m_RotationSpeed, 0.0f, 0.0f, Space::Self);
			Rotate(0.0f, mouseMove.x * dt * m_RotationSpeed, 0.0f, Space::World);
		}
	}
}

void FreeCamera::Controller()
{
	const JoystickState* pState = m_pInput->GetJoystickStateFromIndex(0);
	if (pState)
	{

		float dt = GameTimer::DeltaTime();

		//Moving
		Vector3 moveDirection(0, 0, 0);
		moveDirection.x += pState->GetAxis(ControllerAxis::LEFT_X, 0.2f);
		moveDirection.z -= pState->GetAxis(ControllerAxis::LEFT_Y, 0.2f);
		moveDirection.y -= pState->GetButton(ControllerButton::BUTTON_LB);
		moveDirection.y += pState->GetButton(ControllerButton::BUTTON_RB);


		float moveSpeed = m_MoveSpeed;
		if (pState->GetButton(ControllerButton::BUTTON_LEFT))
		{
			moveSpeed *= m_ShiftMultiplier;
		}
		moveDirection *= dt * moveSpeed;

		if (moveDirection != Vector3())
		{
			Translate(moveDirection, Space::Self);
		}

		//Rotation
		Rotate(5 * pState->GetAxis(ControllerAxis::RIGHT_Y, 0.2f) * dt * m_RotationSpeed, 0.0f, 0.0f, Space::Self);
		Rotate(0.0f, 5 * pState->GetAxis(ControllerAxis::RIGHT_X, 0.2f) * dt * m_RotationSpeed, 0.0f, Space::World);
	}
}
