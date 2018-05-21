#pragma once
#include "Core\Subsystem.h"
#include "InputMapping.h"

class Graphics;

struct JoystickState
{
	std::string Name;
	int Index = -1;
	SDL_Joystick* pJoystick = nullptr;
	SDL_GameController* pController = nullptr;
	SDL_Haptic* pHaptic = nullptr;

	std::vector<bool> Buttons;
	std::vector<bool> ButtonsPressed;
	std::vector<float> Axes;
	std::vector<int> Hats;
};

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

	MulticastDelegate<SDL_Event*>& OnHandleSDL() { return m_OnHandleSDLEvent; }
	MulticastDelegate<int, int>& OnWindowSizeChanged() { return m_OnWindowSizeChangedEvent; }

	const JoystickState* GetJoystickStateFromIndex(int index) const;

	void Rumble(int index);

	static void DrawDebugJoystick(const JoystickState& state);

private:
	void SetKey(int keyCode, bool down);
	void SetMouseButton(int mouseButton, bool down);

	const JoystickState& GetJoystickState(SDL_JoystickID index) const;
	void ResetGamepads();
	bool OpenGamepad(int index);
	bool CloseGamepad(int index);

	MulticastDelegate<SDL_Event*> m_OnHandleSDLEvent;
	MulticastDelegate<int, int> m_OnWindowSizeChangedEvent;

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
	bool m_ForceToCenter;
};