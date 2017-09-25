#include "stdafx.h"

#include "InputEngine.h"
#include "Game/GameManager.h"

InputEngine::InputEngine(): 
	m_Enabled(false), 
	m_ForceToCenter(false)
{
}

InputEngine::~InputEngine()
{
	if(m_pKeyboardState0 != nullptr)
	{
		delete [] m_pKeyboardState0;
		delete [] m_pKeyboardState1;

		m_pKeyboardState0 = nullptr;
		m_pKeyboardState1 = nullptr;
		m_pCurrKeyboardState = nullptr;
		m_pOldKeyboardState = nullptr;
	}
}

void InputEngine::Initialize()
{
	//Static Initialize Check
	if (m_IsInitialized)
		return;

	m_pKeyboardState0 = new BYTE[256];
	m_pKeyboardState1 = new BYTE[256];

	GetKeyboardState(m_pKeyboardState0);
	GetKeyboardState(m_pKeyboardState1);
	
	RefreshControllerConnections();
	m_IsInitialized = true;
}

bool InputEngine::AddInputAction(InputAction action)
{
	if (m_InputActions.find(action.ActionID) == m_InputActions.end())
		m_InputActions[action.ActionID] = vector<InputAction>();

	m_InputActions[action.ActionID].push_back(action); 

	return true;
}

bool InputEngine::IsActionTriggered(int actionID)
{
	for(const InputAction& action : m_InputActions[actionID])
	{
		switch (action.TriggerState)
		{
		case Pressed: 
			if (action.Pressed)
				return true;
			break;
		case Released:
			if (action.Released)
				return true;
			break;
		case Down:
			if (action.Down)
				return true;
			break;
		default:
			break;
		}
	}
	return false;
}

void InputEngine::RefreshControllerConnections()
{
	DWORD dwResult;
	for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		dwResult = XInputGetState(i, &state);
		m_ConnectedGamepads[i] = (dwResult == ERROR_SUCCESS);
	}
}

void InputEngine::UpdateGamepadStates()
{
	DWORD dwResult;
	for(DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		if (!m_ConnectedGamepads[i])
			return;

		m_OldGamepadState[i] = m_CurrGamepadState[i];

		//XINPUT_STATE state;
		//ZeroMemory(&state, sizeof(XINPUT_STATE));

		dwResult = XInputGetState(i, &m_CurrGamepadState[i]);
		m_ConnectedGamepads[i] = (dwResult == ERROR_SUCCESS);
	}
}

bool InputEngine::UpdateKeyboardStates()
{
	//Get Current KeyboardState and set Old KeyboardState
	BOOL getKeyboardResult;
	if(m_KeyboardState0Active)
	{
		getKeyboardResult = GetKeyboardState(m_pKeyboardState1);
		m_pOldKeyboardState = m_pKeyboardState0;
		m_pCurrKeyboardState = m_pKeyboardState1;
	}
	else
	{
		getKeyboardResult = GetKeyboardState(m_pKeyboardState0);
		m_pOldKeyboardState = m_pKeyboardState1;
		m_pCurrKeyboardState = m_pKeyboardState0;
	}

	m_KeyboardState0Active = !m_KeyboardState0Active;

	return getKeyboardResult>0?true:false;
}

void InputEngine::Update()
{
	UpdateKeyboardStates();
	UpdateGamepadStates();

	UpdateKeyboard();

	//Mouse Position
	if (m_MouseMove)
		m_MouseMove = false;
	else
		m_MouseMovement = XMFLOAT2(0, 0);

	m_OldMousePosition = m_CurrMousePosition;
	if(GetCursorPos(&m_CurrMousePosition))
	{
		//ScreenToClient(GameManager::Instance().GetEngineContext().Hwnd, &m_CurrMousePosition);
	}

	if (m_ForceToCenter)
	{
		POINT mouseCenter;
		//m_CurrMousePosition.x = GameManager::Instance().GetEngineContext().GameSettings.Width / 2;
//		m_CurrMousePosition.y = GameManager::Instance().GetEngineContext().GameSettings.Height / 2;
		mouseCenter.x = m_CurrMousePosition.x;
		mouseCenter.y = m_CurrMousePosition.y;
		//ClientToScreen(GameManager::Instance().GetEngineContext().Hwnd, &mouseCenter);

		SetCursorPos(mouseCenter.x, mouseCenter.y);
	}
}

bool InputEngine::IsKeyboardKeyDown(int key) const
{
	return (m_pCurrKeyboardState[key] & 0xF0) != 0 ? true : false;
}

bool InputEngine::IsMouseButtonDown(int button) const
{
	return (m_pCurrKeyboardState[button] & 0xF0) != 0 ? true : false;
}

bool InputEngine::IsGamepadButtonDown(WORD button, GamepadIndex playerIndex) const
{
	return (m_CurrGamepadState[playerIndex].Gamepad.wButtons&button) != 0 ? true : false;
}

XMFLOAT2 InputEngine::GetThumbstickPosition(bool leftThumbstick, GamepadIndex playerIndex) const
{
	XMFLOAT2 pos;
	if(leftThumbstick)
	{
		pos = XMFLOAT2(m_CurrGamepadState[playerIndex].Gamepad.sThumbLX, m_CurrGamepadState[playerIndex].Gamepad.sThumbLY);
		
		if(pos.x>-XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && pos.x<XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)pos.x = 0;
		if(pos.y>-XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && pos.y<XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)pos.y = 0;
	}
	else
	{
		pos = XMFLOAT2(m_CurrGamepadState[playerIndex].Gamepad.sThumbRX, m_CurrGamepadState[playerIndex].Gamepad.sThumbRY);

		if(pos.x>-XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && pos.x<XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)pos.x = 0;
		if(pos.y>-XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && pos.y<XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)pos.y = 0;
	}

	if(pos.x<0)pos.x/=32768;
	else pos.x/=32767;

	if(pos.y<0)pos.y/=32768;
	else pos.y/=32767;
	
	return pos;
}

void InputEngine::UpdateKeyboard()
{
	for (auto it = m_InputActions.begin(); it != m_InputActions.end(); ++it)
	{
		for (InputAction& action : it->second)
		{
			if (action.TriggerState == Pressed)
			{
				if (action.KeyboardCode != -1)
				{
					if (m_pCurrKeyboardState[action.KeyboardCode] & 0xF0 && (m_pOldKeyboardState[action.KeyboardCode] & 0xF0) == false)
						action.Pressed = true;
					else
						action.Pressed = false;
				}
				else if (action.MouseButtonCode != -1)
				{
					if (m_pCurrKeyboardState[action.MouseButtonCode] & 0xF0 && (m_pOldKeyboardState[action.MouseButtonCode] & 0xF0) == false)
						action.Pressed = true;
					else
						action.Pressed = false;
				}
				else if (action.GamepadButtonCode != -1)
				{
					if (m_CurrGamepadState[action.PlayerIndex].Gamepad.wButtons&action.GamepadButtonCode && (m_CurrGamepadState[action.PlayerIndex].Gamepad.wButtons&action.GamepadButtonCode) == 0)
						action.Pressed = true;
					else
						action.Pressed = false;
				}
			}
			else if (action.TriggerState == Down)
			{
				if (action.KeyboardCode != -1)
				{
					if (m_pCurrKeyboardState[action.KeyboardCode] & 0xF0 || m_pOldKeyboardState[action.KeyboardCode] & 0xF0)
						action.Down = true;
					else
						action.Down = false;
				}
				else if (action.MouseButtonCode != -1)
				{
					if (m_pCurrKeyboardState[action.MouseButtonCode] & 0xF0 || m_pOldKeyboardState[action.MouseButtonCode] & 0xF0)
						action.Down = true;
					else
						action.Down = false;
				}
				else if (action.GamepadButtonCode != -1)
				{
					if (m_CurrGamepadState[action.PlayerIndex].Gamepad.wButtons&action.GamepadButtonCode)
						action.Down = true;
					else
						action.Down = false;
				}
			}
			else if (action.TriggerState == Released)
			{
				if (action.KeyboardCode != -1)
				{
					if (m_pOldKeyboardState[action.KeyboardCode] & 0xF0 && (m_pCurrKeyboardState[action.KeyboardCode] & 0xF0) == false)
						action.Released = true;
					else
						action.Released = false;
				}
				else if (action.MouseButtonCode != -1)
				{
					if (m_pOldKeyboardState[action.MouseButtonCode] & 0xF0 && (m_pCurrKeyboardState[action.MouseButtonCode] & 0xF0) == false)
						action.Released = true;
					else
						action.Released = false;
				}
				else if (action.GamepadButtonCode != -1)
				{
					if (m_OldGamepadState[action.PlayerIndex].Gamepad.wButtons&action.GamepadButtonCode && (m_CurrGamepadState[action.PlayerIndex].Gamepad.wButtons&action.GamepadButtonCode) == 0)
						action.Released = true;
					else
						action.Released = false;
				}
			}
		}
	}
}

float InputEngine::GetTriggerPressure(bool leftTrigger, GamepadIndex playerIndex) const
{
	if(leftTrigger)
		return m_CurrGamepadState[playerIndex].Gamepad.bLeftTrigger/255.0f;
	return m_CurrGamepadState[playerIndex].Gamepad.bRightTrigger/255.0f;
}

void InputEngine::SetVibration(float leftVibration, float rightVibration, GamepadIndex playerIndex) const
{
	XINPUT_VIBRATION vibration;
    ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

    vibration.wLeftMotorSpeed = static_cast<WORD>(leftVibration * 65535);
    vibration.wRightMotorSpeed = static_cast<WORD>(rightVibration * 65535);

    XInputSetState(playerIndex, &vibration);
}

void InputEngine::ForceMouseToCenter(bool force)
{
	m_ForceToCenter = force;

	if (force)
	{
		POINT mouseCenter;
		//m_CurrMousePosition.x = GameManager::Instance().GetEngineContext().GameSettings.Width / 2;
		//m_CurrMousePosition.y = GameManager::Instance().GetEngineContext().GameSettings.Height / 2;
		mouseCenter.x = m_CurrMousePosition.x;
		mouseCenter.y = m_CurrMousePosition.y;
		//ClientToScreen(GameManager::Instance().GetEngineContext().Hwnd, &mouseCenter);

		SetCursorPos(mouseCenter.x, mouseCenter.y);
	}
}

