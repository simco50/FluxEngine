#include "stdafx.h"
#include "FluxCore.h"
#include "resource.h"
#include "UI/ImgUIDrawer.h"

using namespace std;

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

FluxCore::FluxCore()
{
}

FluxCore::~FluxCore()
{
	ResourceManager::Release();
	Console::Release();
	ImgUIDrawer::Instance().Shutdown();
	Renderer::Instance().Shutdown();
}

int FluxCore::Run(HINSTANCE hInstance)
{
	Console::Initialize();

	PrepareGame();

	m_hInstance = hInstance;

	HR(RegisterWindowClass());
	HR(MakeWindow());

	RendererDesc desc;
	desc.ClearColor = m_EngineContext.GameSettings.ClearColor;
	desc.Hwnd = m_EngineContext.Hwnd;
	desc.Msaa = m_EngineContext.GameSettings.MSAA;
	desc.VerticalSync = m_EngineContext.GameSettings.VerticalSync;
	desc.WindowHeight = m_EngineContext.GameSettings.Height;
	desc.WindowWidth = m_EngineContext.GameSettings.Width;
	desc.WindowStyle = m_EngineContext.GameSettings.WindowStyle;
	Renderer::Instance().Initialize(desc);

	InitializeHighDefinitionMouse();

	ResourceManager::Initialize(Renderer::Instance().GetDevice());
	InputEngine::Instance().Initialize();
	ImgUIDrawer::Instance().Initialize(m_EngineContext.Hwnd);

	Initialize(&m_EngineContext);

	GameTimer::Reset();

	//Game loop
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if(!GameTimer::IsPaused())
				GameLoop();
			else
				Sleep(100);
		}
	}
	return msg.wParam;
}

void FluxCore::GameLoop()
{
	GameTimer::Tick();
	CalculateFrameStats();

	AudioEngine::Instance().Update();
	InputEngine::Instance().Update();

	Update();

	Renderer::Instance().NewFrame();
	ImgUIDrawer::Instance().NewFrame();
	Renderer::Instance().Render();
	ImgUIDrawer::Instance().Render();
	Renderer::Instance().Present();
}

HRESULT FluxCore::RegisterWindowClass()
{
	WNDCLASSA wc;

	wc.hInstance = m_hInstance;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	HICON icon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hIcon = icon;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpfnWndProc = WndProcStatic;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = m_WindowClassName.c_str();
	wc.lpszMenuName = nullptr;

	if (!RegisterClassA(&wc))
	{
		auto error = GetLastError();
		return HRESULT_FROM_WIN32(error);
	}
	return S_OK;
}

HRESULT FluxCore::MakeWindow()
{
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if(m_EngineContext.GameSettings.WindowStyle == WindowStyle::BORDERLESS)
	{
		m_EngineContext.GameSettings.Width = screenWidth;
		m_EngineContext.GameSettings.Height = screenHeight;
		windowStyle = WS_POPUP;
	}

	RECT windowRect = { 0, 0, m_EngineContext.GameSettings.Width, m_EngineContext.GameSettings.Height };
	AdjustWindowRect(&windowRect, windowStyle, false);

	int x = (screenWidth - windowRect.right) / 2;
	int y = (screenHeight - windowRect.bottom) / 2;

	m_EngineContext.Hwnd = CreateWindowA(
		m_WindowClassName.c_str(),
		m_EngineContext.GameSettings.Title.c_str(),
		windowStyle,
		x,
		y,
		windowRect.right,
		windowRect.bottom,
		nullptr,
		nullptr,
		m_hInstance,
		this
	);

	if(m_EngineContext.Hwnd == nullptr)
	{
		auto error = GetLastError();
		return HRESULT_FROM_WIN32(error);
	}

	ShowWindow(m_EngineContext.Hwnd, SW_SHOW);
	UpdateWindow(m_EngineContext.Hwnd);

	return S_OK;
}

void FluxCore::InitializeHighDefinitionMouse()
{
	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = m_EngineContext.Hwnd;
	RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
}

LRESULT CALLBACK FluxCore::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CREATE)
	{
		CREATESTRUCT *pCS = reinterpret_cast<CREATESTRUCT*>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG>(pCS->lpCreateParams));
	}
	else
	{
		FluxCore* pThisGame = reinterpret_cast<FluxCore*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
		if (pThisGame) return pThisGame->WndProc(hWnd, message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT FluxCore::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		OnPause(LOWORD(wParam) == WA_INACTIVE);
		return 0;

		// WM_SIZE is sent when the user resizes the window.
	case WM_SIZE:
	{
		// Save the new client area dimensions.
		int newWidth = LOWORD(lParam);
		int newHeight = HIWORD(lParam);
		m_EngineContext.GameSettings.Width = newWidth;
		m_EngineContext.GameSettings.Height = newHeight;
		if (Renderer::Instance().GetDevice() != nullptr)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				OnPause(true);
				m_Minimized = true;
				m_Maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				OnPause(false);
				m_Minimized = false;
				m_Maximized = true;
				Renderer::Instance().OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				// Restoring from minimized state?
				if (m_Minimized)
				{
					OnPause(false);
					m_Minimized = false;
					Renderer::Instance().OnResize();
				}
				// Restoring from maximized state?
				else if (m_Maximized)
				{
					OnPause(false);
					m_Maximized = false;
					Renderer::Instance().OnResize();
				}
				else if (!m_Resizing) // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					Renderer::Instance().OnResize();
				}
			}
		}
		return 0;
	}

	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		OnPause(true);
		m_Resizing = true;
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		OnPause(false);
		m_Resizing = false;
		Renderer::Instance().OnResize();
		return 0;

	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	//High-Definition Mouse Movement
	case WM_INPUT:
	{
		UINT dwSize = 40;
		static BYTE lpb[40];

		GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
			lpb, &dwSize, sizeof(RAWINPUTHEADER));

		RAWINPUT* raw = (RAWINPUT*)lpb;

		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			float xPosRelative = (float)raw->data.mouse.lLastX;
			float yPosRelative = (float)raw->data.mouse.lLastY;
			InputEngine::Instance().SetMouseMovement(XMFLOAT2(xPosRelative, yPosRelative));
		}
		return 0;
	}

	default:
		break;
	}

	ImgUIDrawer::Instance().WndProc(message, wParam, lParam);

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void FluxCore::CalculateFrameStats() const
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((GameTimer::GameTime() - timeElapsed) >= 1.0f)
	{
		int fps = frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / (float)fps;

		stringstream str;
		str << m_EngineContext.GameSettings.Title << "\t FPS: " << fps << "\t MS: " << mspf;
		string title = str.str();
		SetWindowTextA(m_EngineContext.Hwnd, str.str().c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void FluxCore::OnPause(const bool paused)
{
	if (paused)
		GameTimer::Stop();
	else
		GameTimer::Start();
}