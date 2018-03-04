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
}

InputEngine::~InputEngine()
{
}

bool InputEngine::AddInputAction(InputAction action)
{
	if (m_InputActions.find(action.ActionID) == m_InputActions.end())
		m_InputActions[action.ActionID] = std::vector<InputAction>();

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

void InputEngine::Update()
{
	ZeroMemory(m_KeyPressed.data(), m_KeyPressed.size() * sizeof(bool));
	m_MouseButtonPressed = 0;

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
				m_pGraphics->OnResize();
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
		}
		/*case SDL_KEYDOWN:
			SetKey(SDL_toupper(event.key.keysym.sym), true);
			break;
		case SDL_KEYUP:
			SetKey(SDL_toupper(event.key.keysym.sym), false);
			break;*/
		case SDL_MOUSEBUTTONDOWN:
			SetMouseButton(1 << (event.button.button - 1), true);
			break;
		case SDL_MOUSEBUTTONUP:
			SetMouseButton(1 << (event.button.button - 1), false);
			break;
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
	m_CurrMousePosition.x = mousePosX;
	m_CurrMousePosition.y = mousePosY;

	m_MouseMovement.x = (float)m_CurrMousePosition.x - (float)m_OldMousePosition.x;
	m_MouseMovement.y = (float)m_CurrMousePosition.y - (float)m_OldMousePosition.y;
}

void InputEngine::CursorVisible(bool visible) const
{
	SDL_ShowCursor((int)visible);
}

bool InputEngine::IsKeyboardKeyDown(int key) const
{
	return m_KeyDown[key];
}

bool InputEngine::IsKeyboardKeyPressed(int key) const
{
	return m_KeyPressed[key];
}

bool InputEngine::IsMouseButtonDown(int button) const
{
	return ((m_MouseButtonDown >> (button - 1)) & 1) == 1;
}

bool InputEngine::IsMouseButtonPressed(int button) const
{
	return ((m_MouseButtonPressed >> (button - 1)) & 1) == 1;
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

void InputEngine::ForceMouseToCenter(bool force)
{
	SDL_SetRelativeMouseMode(force ? SDL_TRUE : SDL_FALSE);
}