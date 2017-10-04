#pragma once
#include <map>

class Graphics;

enum GamepadIndex: DWORD
{
	PlayerOne = 0,
	PlayerTwo = 1,
	PlayerThree = 2,
	PlayerFour = 3
};

enum InputTriggerState
{
	Pressed,
	Released,
	Down
};

struct InputAction
{
	InputAction():
		ActionID(-1),
		TriggerState(InputTriggerState::Pressed), 
		KeyboardCode(-1), 
		MouseButtonCode(-1),
		GamepadButtonCode(0),
		PlayerIndex(GamepadIndex::PlayerOne)
		{}

	InputAction(int actionID, InputTriggerState triggerState = InputTriggerState::Pressed, int keyboardCode = -1, int mouseButtonCode = -1, WORD gamepadButtonCode = 0, GamepadIndex playerIndex = GamepadIndex::PlayerOne):
		ActionID(actionID),
		TriggerState(triggerState), 
		KeyboardCode(keyboardCode), 
		MouseButtonCode(mouseButtonCode),
		GamepadButtonCode(gamepadButtonCode),
		PlayerIndex(playerIndex)
		{}

	int ActionID;
	InputTriggerState TriggerState;
	int KeyboardCode; //VK_... (Range 0x07 <> 0xFE)
	int MouseButtonCode; //VK_... (Range 0x00 <> 0x06)
	WORD GamepadButtonCode; //XINPUT_GAMEPAD_...
	GamepadIndex PlayerIndex;
	bool Pressed = false;
	bool Released = false;
	bool Down = false;
};

class InputEngine
{
public:

	InputEngine(Graphics* pGraphics);
	~InputEngine();

	InputEngine(const InputEngine& t) = delete;
	InputEngine& operator=(const InputEngine& t) = delete;

	void Update();
	bool AddInputAction(InputAction action);
	bool IsActionTriggered(int actionID);
	void ForceMouseToCenter(bool force);
	void SetEnabled(bool enabled) { m_Enabled = enabled; }

	POINT GetMousePosition(bool previousFrame = false) const {return (previousFrame)?m_OldMousePosition:m_CurrMousePosition; }
	XMFLOAT2 GetMouseMovement() const { return m_MouseMovement; }
	void SetMouseMovement(const XMFLOAT2 &mouseMovement) { m_MouseMove = true;  m_MouseMovement = mouseMovement; }
	XMFLOAT2 GetThumbstickPosition(bool leftThumbstick = true, GamepadIndex playerIndex = GamepadIndex::PlayerOne) const;
	float GetTriggerPressure(bool leftTrigger = true, GamepadIndex playerIndex = GamepadIndex::PlayerOne) const;
	void SetVibration(float leftVibration, float rightVibration, GamepadIndex playerIndex = GamepadIndex::PlayerOne) const;

	void CursorVisible(bool visible) const { ShowCursor(visible); }
	bool IsGamepadConnected(GamepadIndex index) const { return m_ConnectedGamepads[(DWORD)index]; }

	bool IsKeyboardKeyDown(int key) const;
	bool IsKeyboardKeyPressed(int key) const;
	bool IsMouseButtonDown(int button) const;
	bool IsGamepadButtonDown(WORD button, GamepadIndex playerIndex = GamepadIndex::PlayerOne) const;
	void RefreshControllerConnections();

	const BYTE* GetKeyboardFlags() const { return m_pCurrKeyboardState; }

private:
	Graphics* m_pGraphics = nullptr;

	map<int, vector<InputAction>> m_InputActions;
	BYTE *m_pCurrKeyboardState = nullptr;
	BYTE *m_pOldKeyboardState = nullptr;
	BYTE *m_pKeyboardState0 = nullptr;
	BYTE *m_pKeyboardState1 = nullptr;
	bool m_KeyboardState0Active = false;
	POINT m_CurrMousePosition;
	POINT m_OldMousePosition;
	bool m_MouseMove;
	XMFLOAT2 m_MouseMovement;

	XINPUT_STATE m_OldGamepadState[XUSER_MAX_COUNT], m_CurrGamepadState[XUSER_MAX_COUNT];
	bool m_ConnectedGamepads[XUSER_MAX_COUNT];
	bool m_Enabled;
	bool m_ForceToCenter;

	void UpdateGamepadStates();
	bool UpdateKeyboardStates();
	void UpdateKeyboard();
};