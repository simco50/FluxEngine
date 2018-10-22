#pragma once
#include "Core\Subsystem.h"
#include "InputMapping.h"

class Graphics;

class JoystickState
{
public:
	static const int MAX_BUTTONS = SDL_CONTROLLER_BUTTON_MAX;
	static const int MAX_AXIS = SDL_CONTROLLER_BUTTON_MAX;
	static const int MAX_HATS = 4;

	float GetButton(ControllerButton axis) const
	{
		return Buttons[(int)axis];
	}
	float GetButtonDown(ControllerButton axis) const
	{
		return ButtonsPressed[(int)axis];
	}
	float GetAxis(ControllerAxis axis) const
	{
		return Axes[(int)axis];
	}
	float GetAxis(ControllerAxis axis, float deadZone) const
	{
		if (abs(Axes[(int)axis]) < deadZone)
		{
			return 0;
		}
		return Axes[(int)axis];
	}

private:
	friend class InputEngine;

	std::string Name;
	int Index = -1;
	SDL_Joystick* pJoystick = nullptr;
	SDL_GameController* pController = nullptr;
	SDL_Haptic* pHaptic = nullptr;

	bool Buttons[MAX_BUTTONS];
	bool ButtonsPressed[MAX_BUTTONS];
	float Axes[MAX_AXIS];
	int Hats[MAX_HATS];
};

DECLARE_MULTICAST_DELEGATE(OnSDLEvent, SDL_Event*);
DECLARE_MULTICAST_DELEGATE(OnWindowSizeChangedDelegate, int, int);

class InputEngine : public Subsystem
{
	FLUX_OBJECT(InputEngine, Subsystem)

public:
	InputEngine(Context* pContext);
	~InputEngine();

	DELETE_COPY(InputEngine)

	void Update();
	void ForceMouseToCenter(bool force);
	void SetEnabled(bool enabled) { m_Enabled = enabled; }

	const Vector2& GetMousePosition(bool previousFrame = false) const {return (previousFrame)?m_OldMousePosition:m_CurrMousePosition; }
	const Vector2& GetMouseMovement() const { return m_MouseMovement; }
	int GetMouseWheel() const { return m_MouseWheel; }
	void SetMouseMovement(const Vector2 &mouseMovement) { m_MouseMove = true;  m_MouseMovement = mouseMovement; }

	void CursorVisible(bool visible) const;

	bool IsKeyboardKeyDown(KeyboardKey key) const;
	bool IsKeyboardKeyPressed(KeyboardKey key) const;
	bool IsMouseButtonDown(MouseKey button) const;
	bool IsMouseButtonPressed(MouseKey button) const;

	OnSDLEvent& OnHandleSDL() { return m_OnHandleSDLEvent; }
	OnWindowSizeChangedDelegate& OnWindowSizeChanged() { return m_OnWindowSizeChangedEvent; }

	const JoystickState* GetJoystickStateFromIndex(int index) const;

	bool Rumble(const int index, const float strength, const unsigned int length);

	void DrawDebugJoysticks();

private:
	void SetKey(int keyCode, bool down);
	void SetMouseButton(int mouseButton, bool down);

	const JoystickState& GetJoystickState(SDL_JoystickID index) const;
	void ResetGamepads();
	bool OpenGamepad(int index);
	bool CloseGamepad(int index);
	void CloseAllGamepads();

	OnSDLEvent m_OnHandleSDLEvent;
	OnWindowSizeChangedDelegate m_OnWindowSizeChangedEvent;

	Graphics * m_pGraphics = nullptr;

	Vector2 m_CurrMousePosition = {};
	Vector2 m_OldMousePosition = {};
	bool m_MouseMove = false;
	Vector2 m_MouseMovement;

	unsigned int m_MouseButtonDown = 0;
	unsigned int m_MouseButtonPressed = 0;
	int m_MouseWheel = 0;

	std::array<bool, (size_t)KeyboardKey::MAX> m_KeyDown = {};
	std::array<bool, (size_t)KeyboardKey::MAX> m_KeyPressed = {};

	std::unordered_map<SDL_JoystickID, JoystickState> m_Joysticks;

	bool m_Enabled;
	bool m_PauseOnInactive = true;
};