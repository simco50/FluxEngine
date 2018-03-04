#pragma once
#include "Core\Subsystem.h"

class Graphics;

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
		MouseButtonCode(-1)
		{}

	InputAction(int actionID, InputTriggerState triggerState = InputTriggerState::Pressed, int keyboardCode = -1, int mouseButtonCode = -1):
		ActionID(actionID),
		TriggerState(triggerState), 
		KeyboardCode(keyboardCode), 
		MouseButtonCode(mouseButtonCode)
		{}

	int ActionID;
	InputTriggerState TriggerState;
	int KeyboardCode; //VK_... (Range 0x07 <> 0xFE)
	int MouseButtonCode; //VK_... (Range 0x00 <> 0x06)
	WORD GamepadButtonCode; //XINPUT_GAMEPAD_...
	bool Pressed = false;
	bool Released = false;
	bool Down = false;
};

class InputEngine : public Subsystem
{
	FLUX_OBJECT(InputEngine, Subsystem)

public:
	InputEngine(Context* pContext);
	~InputEngine();

	DELETE_COPY(InputEngine)

	void Update();
	bool AddInputAction(InputAction action);
	bool IsActionTriggered(int actionID);
	void ForceMouseToCenter(bool force);
	void SetEnabled(bool enabled) { m_Enabled = enabled; }

	POINT GetMousePosition(bool previousFrame = false) const {return (previousFrame)?m_OldMousePosition:m_CurrMousePosition; }
	const Vector2& GetMouseMovement() const { return m_MouseMovement; }
	void SetMouseMovement(const Vector2 &mouseMovement) { m_MouseMove = true;  m_MouseMovement = mouseMovement; }

	void CursorVisible(bool visible) const;

	bool IsKeyboardKeyDown(int key) const;
	bool IsKeyboardKeyPressed(int key) const;
	bool IsMouseButtonDown(int button) const;
	bool IsMouseButtonPressed(int button) const;

	MulticastDelegate<SDL_Event*>& OnHandleSDL() { return m_OnHandleSDLEvent; }

private:
	void SetKey(int keyCode, bool down);
	void SetMouseButton(int mouseButton, bool down);

	MulticastDelegate<SDL_Event*> m_OnHandleSDLEvent;

	Graphics * m_pGraphics = nullptr;

	std::map<int, std::vector<InputAction>> m_InputActions;
	
	POINT m_CurrMousePosition;
	POINT m_OldMousePosition;
	bool m_MouseMove = false;
	Vector2 m_MouseMovement;

	unsigned int m_MouseButtonDown = 0;
	unsigned int m_MouseButtonPressed = 0;
	std::array<bool, 256> m_KeyDown = {};
	std::array<bool, 256> m_KeyPressed = {};

	bool m_Enabled;
	bool m_ForceToCenter;
};