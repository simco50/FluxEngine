#include "FluxEngine.h"
#include "InputEngine.h"
#include "Rendering/Core/Graphics.h"
#include "Core/FluxCore.h"

InputEngine::InputEngine(Context* pContext) :
	Subsystem(pContext),
	m_Enabled(false), 
	m_ForceToCenter(false)
{
	m_pGraphics = pContext->GetSubsystem<Graphics>();
	pContext->InitSDLSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);

	ResetGamepads();
}

InputEngine::~InputEngine()
{
	m_pContext->ShutdownSDL();
}

void InputEngine::Update()
{
	ZeroMemory(m_KeyPressed.data(), m_KeyPressed.size() * sizeof(bool));
	m_MouseButtonPressed = 0;
	m_MouseWheel = 0;

	for (auto& joystickState : m_Joysticks)
	{
		for (size_t i = 0; i < joystickState.second.ButtonsPressed.size(); ++i)
		{
			joystickState.second.ButtonsPressed[i] = false;
		}
	}

	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_WINDOWEVENT:
		{
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
				m_pGraphics->OnResize(event.window.data1, event.window.data2);
				m_OnWindowSizeChangedEvent.Broadcast(event.window.data1, event.window.data2);
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
			case SDL_WINDOWEVENT_MINIMIZED:
				GameTimer::Stop();
				break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			case SDL_WINDOWEVENT_MAXIMIZED:
				GameTimer::Start();
				break;
			}
			break;
		}
		case SDL_KEYDOWN:
			SetKey(event.key.keysym.scancode, true);
			break;
		case SDL_KEYUP:
			SetKey(event.key.keysym.scancode, false);
			break;
		case SDL_MOUSEBUTTONDOWN:
			SetMouseButton(1 << (event.button.button - 1), true);
			break;
		case SDL_MOUSEBUTTONUP:
			SetMouseButton(1 << (event.button.button - 1), false);
			break;
		case SDL_MOUSEWHEEL:
			m_MouseWheel += event.wheel.y;
			break;

		case SDL_JOYDEVICEADDED:
			OpenGamepad(event.jdevice.which);
			break;
		case SDL_JOYDEVICEREMOVED:
			m_Joysticks.erase(event.jdevice.which);
			break;
		case SDL_JOYBUTTONDOWN:
		{
			int button = event.jbutton.button;
			SDL_JoystickID joystickId = event.jbutton.which;
			JoystickState& state = m_Joysticks[joystickId];
			if (!state.pController)
			{
				if (button < (int)state.Buttons.size())
				{
					state.Buttons[button] = true;
					state.ButtonsPressed[button] = true;
				}
			}
			break;
		}
		case SDL_JOYBUTTONUP:
		{
			int button = event.jbutton.button;
			SDL_JoystickID joystickId = event.jbutton.which;
			JoystickState& state = m_Joysticks[joystickId];
			if (!state.pController)
			{
				if (button < (int)state.Buttons.size())
				{
					state.Buttons[button] = false;
				}
			}
			break;
		}
		case SDL_JOYAXISMOTION:
		{
			SDL_JoystickID joystickId = event.jaxis.which;
			JoystickState& state = m_Joysticks[joystickId];

			if (!state.pController)
			{
				if (event.jaxis.axis < (int)state.Axes.size())
				{
					state.Axes[event.jaxis.axis] = Math::Clamp((float)event.jaxis.value / 32767.0f, 1.0f, -1.0f);
				}
			}
			break;
		}
		case SDL_JOYHATMOTION:
		{
			SDL_JoystickID joystickId = event.jhat.which;
			JoystickState& state = m_Joysticks[joystickId];

			if (!state.pController)
			{
				if (event.jhat.which < (int)state.Hats.size())
				{
					state.Hats[event.jhat.which] = event.jhat.value;
				}
			}
			break;
		}
		case SDL_CONTROLLERBUTTONDOWN:
		{
			int button = event.cbutton.button;
			SDL_JoystickID joystickId = event.cbutton.which;
			JoystickState& state = m_Joysticks[joystickId];
			if (button < (int)state.Buttons.size())
			{
				state.Buttons[button] = true;
				state.ButtonsPressed[button] = true;
			}
			break;
		}
		case SDL_CONTROLLERBUTTONUP:
		{
			int button = event.cbutton.button;
			SDL_JoystickID joystickId = event.cbutton.which;
			JoystickState& state = m_Joysticks[joystickId];
			if (button < (int)state.Buttons.size())
			{
				state.Buttons[button] = false;
			}
			break;
		}
		case SDL_CONTROLLERAXISMOTION:
		{
			SDL_JoystickID joystickId = event.caxis.which;
			JoystickState& state = m_Joysticks[joystickId];

			if (event.caxis.axis < (int)state.Axes.size())
			{
				state.Axes[event.caxis.axis] = Math::Clamp((float)event.caxis.value / 32767.0f, 1.0f, -1.0f);
			}
			break;
		}
		case SDL_QUIT:
			FluxCore::DoExit();
			break;
		}
		m_OnHandleSDLEvent.Broadcast(&event);
	}

	//Mouse Position
	if (m_MouseMove)
		m_MouseMove = false;
	else
		m_MouseMovement = Vector2(0, 0);

	m_OldMousePosition = m_CurrMousePosition;
	int mousePosX, mousePosY;
	SDL_GetMouseState(&mousePosX, &mousePosY);
	m_CurrMousePosition.x = (float)mousePosX;
	m_CurrMousePosition.y = (float)mousePosY;

	m_MouseMovement.x = (float)m_CurrMousePosition.x - (float)m_OldMousePosition.x;
	m_MouseMovement.y = (float)m_CurrMousePosition.y - (float)m_OldMousePosition.y;
}

void InputEngine::CursorVisible(bool visible) const
{
	SDL_ShowCursor((int)visible);
}

bool InputEngine::IsKeyboardKeyDown(KeyboardKey key) const
{
	return m_KeyDown[(int)key];
}

bool InputEngine::IsKeyboardKeyPressed(KeyboardKey key) const
{
	return m_KeyPressed[(int)key];
}

bool InputEngine::IsMouseButtonDown(MouseKey button) const
{
	return ((m_MouseButtonDown >> ((int)button - 1)) & 1) == 1;
}

bool InputEngine::IsMouseButtonPressed(MouseKey button) const
{
	return ((m_MouseButtonPressed >> ((int)button - 1)) & 1) == 1;
}

void InputEngine::SetKey(int keyCode, bool down)
{
	if (down)
	{
		if (m_KeyDown[keyCode] == false)
			m_KeyPressed[keyCode] = true;
		m_KeyDown[keyCode] = true;
	}
	else
	{
		m_KeyDown[keyCode] = false;
	}
}

void InputEngine::SetMouseButton(int mouseButton, bool down)
{
	if (down)
	{
		if ((m_MouseButtonDown & mouseButton) == 0)
		{
			m_MouseButtonPressed |= mouseButton;
		}
		m_MouseButtonDown |= mouseButton;
	}
	else
	{
		m_MouseButtonDown &= ~mouseButton;
	}
}

const JoystickState& InputEngine::GetJoystickState(SDL_JoystickID index) const
{
	return m_Joysticks.at(index);
}

const JoystickState* InputEngine::GetJoystickStateFromIndex(int index) const
{
	for (const auto& pair : m_Joysticks)
	{
		if (pair.second.Index == index)
		{
			return &pair.second;
		}
	}
	return nullptr;
}

void InputEngine::Rumble(int index)
{
	const JoystickState* pState = GetJoystickStateFromIndex(index);
	SDL_HapticRumblePlay(pState->pHaptic, 1, 200);
}

ImVec2 operator+(const ImVec2& a, const ImVec2& b)
{
	return ImVec2(a.x + b.x, a.y + b.y);
}

void InputEngine::DrawDebugJoystick(const JoystickState& state)
{
	ImGui::Begin("Joystick", nullptr, ImVec2(500, 200), -1.0f, ImGuiWindowFlags_NoResize);
	ImGui::Text("Jostick: %i - %s", state.Index, state.Name.c_str());
	ImVec2 basePos = ImGui::GetWindowPos() + ImVec2(70, 120);
	ImGui::GetWindowDrawList()->AddCircle(basePos, 50, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)), 20, 20.0f);
	ImGui::GetWindowDrawList()->AddCircleFilled(basePos, 5, ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.5f, 0.5f, 1)), 12);
	ImGui::GetWindowDrawList()->AddCircleFilled(basePos + ImVec2(state.Axes[0] * 45, state.Axes[1] * 45), 5, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)));

	basePos = basePos + ImVec2(140, 0);
	ImGui::GetWindowDrawList()->AddCircle(basePos, 50, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)), 20, 20.0f);
	ImGui::GetWindowDrawList()->AddCircleFilled(basePos, 5, ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.5f, 0.5f, 1)), 12);
	ImGui::GetWindowDrawList()->AddCircleFilled(basePos + ImVec2(state.Axes[2] * 45, state.Axes[3] * 45), 5, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)));

	basePos = basePos + ImVec2(80, -70);
	ImGui::GetWindowDrawList()->AddRect(basePos, basePos + ImVec2(20, 140), ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)), 0, ImDrawCornerFlags_All, 10);
	ImGui::GetWindowDrawList()->AddRectFilled(basePos, basePos + ImVec2(20, state.Axes[4] * 140), ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)));
	basePos = basePos + ImVec2(40, 0);
	ImGui::GetWindowDrawList()->AddRect(basePos, basePos + ImVec2(20, 140), ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)), 0, ImDrawCornerFlags_All, 10);
	ImGui::GetWindowDrawList()->AddRectFilled(basePos, basePos + ImVec2(20, state.Axes[5] * 140), ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)));


	basePos = basePos + ImVec2(100, 110);
	ImGui::GetWindowDrawList()->AddCircle(basePos, 10, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)), 12, 4.0f);
	if (state.Buttons[0])
	{
		ImGui::GetWindowDrawList()->AddCircleFilled(basePos, 10, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)), 12);
	}
	basePos = basePos + ImVec2(0, -80);
	ImGui::GetWindowDrawList()->AddCircle(basePos, 10, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)), 12, 4.0f);
	if (state.Buttons[3])
	{
		ImGui::GetWindowDrawList()->AddCircleFilled(basePos, 10, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)), 12);
	}
	basePos = basePos + ImVec2(-40, 40);
	ImGui::GetWindowDrawList()->AddCircle(basePos, 10, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)), 12, 4.0f);
	if (state.Buttons[2])
	{
		ImGui::GetWindowDrawList()->AddCircleFilled(basePos, 10, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)), 12);
	}
	basePos = basePos + ImVec2(80, 0);
	ImGui::GetWindowDrawList()->AddCircle(basePos, 10, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)), 12, 4.0f);
	if (state.Buttons[1])
	{
		ImGui::GetWindowDrawList()->AddCircleFilled(basePos, 10, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)), 12);
	}

	ImGui::End();
}

void InputEngine::ResetGamepads()
{
	m_Joysticks.clear();
	int size = SDL_NumJoysticks();
	for (int i = 0; i < size; ++i)
	{
		OpenGamepad(i);
	}
}

bool InputEngine::OpenGamepad(int index)
{
	SDL_Joystick* pJoystick = SDL_JoystickOpen(index);
	if (pJoystick == nullptr)
	{
		return false;
	}
	SDL_JoystickID instanceId = SDL_JoystickInstanceID(pJoystick);

	JoystickState& state = m_Joysticks[instanceId];
	state.Index = index;
	state.Name = SDL_JoystickName(pJoystick);
	state.pJoystick = pJoystick;

	if (SDL_IsGameController(index))
	{
		state.pController = SDL_GameControllerOpen(index);
		if (state.pController == nullptr)
		{
			FLUX_LOG(Warning, "[InputEngine::OpenGamepad] Failed to open GameController: %s", SDL_GetError());
		}
	}

	state.pHaptic = SDL_HapticOpenFromJoystick(pJoystick);
	if (state.pHaptic)
	{
		if (SDL_HapticRumbleInit(state.pHaptic) < 0)
		{
			FLUX_LOG(Warning, "[InputEngine::OpenGamepad] Failed to intialize rumble: %s", SDL_GetError());
		}
	}

	unsigned numButtons = (unsigned)SDL_JoystickNumButtons(pJoystick);
	unsigned numAxes = (unsigned)SDL_JoystickNumAxes(pJoystick);
	unsigned numHats = (unsigned)SDL_JoystickNumHats(pJoystick);

	if (state.pController)
	{
		if (numButtons < SDL_CONTROLLER_BUTTON_MAX)
			numButtons = SDL_CONTROLLER_BUTTON_MAX;
		if (numAxes < SDL_CONTROLLER_AXIS_MAX)
			numAxes = SDL_CONTROLLER_AXIS_MAX;
	}

	state.Buttons.resize(numButtons);
	state.ButtonsPressed.resize(numButtons);
	state.Axes.resize(numAxes);
	state.Hats.resize(numHats);

	return true;
}

bool InputEngine::CloseGamepad(int index)
{
	for (auto& pair : m_Joysticks)
	{
		if (pair.second.Index == index)
		{
			SDL_JoystickClose(pair.second.pJoystick);
			if (pair.second.pController)
			{
				SDL_GameControllerClose(pair.second.pController);
			}
			if (pair.second.pHaptic)
			{
				SDL_HapticClose(pair.second.pHaptic);
			}
			m_Joysticks.erase(pair.first);
			return true;
		}
	}
	return false;
}

void InputEngine::ForceMouseToCenter(bool force)
{
	SDL_SetRelativeMouseMode(force ? SDL_TRUE : SDL_FALSE);
}