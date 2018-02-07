#include "FluxEngine.h"
#include "Window.h"

Window::Window(
	const int width, 
	const int height, 
	const std::string& title /*= "Window"*/, 
	const WindowType type/*= WindowType::WINDOWED*/, 
	const bool resizable /*= true*/, 
	const std::string& className /*= "windowClass"*/) :
	m_Width(width),
	m_Height(height),
	m_Title(title),
	m_Type(type),
	m_Resizable(resizable),
	m_ClassName(className)
{
}

Window::~Window()
{
	if (m_Hwnd)
		DestroyWindow(m_Hwnd);
}

bool Window::Open()
{
	AUTOPROFILE(Window_Open);
	if (!CreateClass(m_ClassName))
		return false;
	if (!CreateInstanceOfClass(m_ClassName, m_Width, m_Height))
		return false;
	FLUX_LOG(INFO, "[Window::Open] > Window created");
	return true;
}

void Window::SetIcon(const std::string& iconPath)
{
	HANDLE icon = LoadImage(nullptr, iconPath.c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	SendMessage(m_Hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
}

void Window::SetTitle(const string& title)
{
	SetWindowText(m_Hwnd, title.c_str());
}

void Window::SetPosition(const int x, const int y)
{
	
	SetWindowPos(m_Hwnd, HWND_TOP, x, y, -1, -1, SWP_NOSIZE);
}

bool Window::CreateClass(const std::string& className)
{
	AUTOPROFILE_DESC(Window_CreateClass, className.c_str());

	WNDCLASSA wc;

	wc.hInstance = GetModuleHandle(0);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpfnWndProc = WndProcStatic;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = className.c_str();
	wc.lpszMenuName = nullptr;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	if (!RegisterClassA(&wc))
	{
		auto error = GetLastError();
		FLUX_LOG_HR("[Window::CreateClass()]", HRESULT_FROM_WIN32(error));
		return false;
	}
	return true;
}

bool Window::CreateInstanceOfClass(const std::string& className, int windowWidth, int windowHeight)
{
	AUTOPROFILE_DESC(Graphics_MakeWindow, className.c_str());

	int displayWidth = GetSystemMetrics(SM_CXSCREEN);
	int displayHeight = GetSystemMetrics(SM_CYSCREEN);

	DWORD windowStyle = WS_VISIBLE;
	switch (m_Type)
	{
	case WindowType::FULLSCREEN:
		windowWidth = displayWidth;
		windowHeight = displayHeight;
		break;
	case WindowType::WINDOWED:
		windowStyle =
			WS_OVERLAPPED
			| WS_MINIMIZEBOX
			| WS_SYSMENU
			| WS_CAPTION;
		break;
	case WindowType::BORDERLESS:
		windowWidth = displayWidth;
		windowHeight = displayHeight;
		windowStyle = WS_POPUP;
		break;
	}
	if (m_Resizable)
		windowStyle |= WS_MAXIMIZEBOX | WS_THICKFRAME;

	RECT windowRect = { 0, 0, windowWidth, windowHeight };
	AdjustWindowRect(&windowRect, windowStyle, false);
	windowWidth = windowRect.right - windowRect.left;
	windowHeight = windowRect.bottom - windowRect.top;

	int x = (displayWidth - windowWidth) / 2;
	int y = (displayHeight - windowHeight) / 2;

	m_Hwnd = CreateWindow(
		className.c_str(),
		m_Title.c_str(),
		windowStyle,
		x,
		y,
		windowWidth,
		windowHeight,
		nullptr,
		nullptr,
		GetModuleHandle(0),
		this
	);

	if (m_Hwnd == nullptr)
		return false;

	ShowWindow(m_Hwnd, SW_SHOWDEFAULT);
	if (!UpdateWindow(m_Hwnd))
		return false;

	return true;
}

LRESULT CALLBACK Window::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* pThis;

	if (message == WM_NCCREATE)
	{
		pThis = static_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		SetLastError(0);
		if (!SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis)))
		{
			if (GetLastError() != 0)
				return 0;
		}
	}
	else
	{
		pThis = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}
	if (pThis)
	{
		return pThis->WndProc(hWnd, message, wParam, lParam);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	m_OnWndProcEvent.Broadcast(hWnd, message, wParam, lParam);
		
	switch (message)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		m_Active = !(LOWORD(wParam) == WA_INACTIVE || m_Minimized);
		m_OnWindowStateChangedEvent.Broadcast(m_Active);
		return 0;

		// WM_SIZE is sent when the user resizes the window.
	case WM_SIZE:
	{
		// Save the new client area dimensions.
		m_Width = LOWORD(lParam);
		m_Height = HIWORD(lParam);
		if (wParam == SIZE_MINIMIZED)
		{
			m_Minimized = true;
			m_Maximized = false;
			m_Active = true;
			m_OnWindowStateChangedEvent.Broadcast(m_Active);
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			m_Minimized = false;
			m_Maximized = true;
			m_Active = true;
			m_OnWindowStateChangedEvent.Broadcast(m_Active);
			m_OnWindowSizeChangedEvent.Broadcast(m_Width, m_Height);
		}
		else if (wParam == SIZE_RESTORED)
		{
			m_Active = true;
			if (m_Minimized)
			{
				m_Minimized = false;
				m_OnWindowSizeChangedEvent.Broadcast(m_Width, m_Height);
			}
			else if (m_Maximized)
			{
				m_Maximized = false;
				m_OnWindowSizeChangedEvent.Broadcast(m_Width, m_Height);
			}
			else if (!m_Resizing)
			{
				m_OnWindowSizeChangedEvent.Broadcast(m_Width, m_Height);
			}
		}
		return 0;
	}

	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		if (!m_Resizing)
		{
			m_Resizing = true;
			m_Active = false;
			m_OnWindowStateChangedEvent.Broadcast(m_Active);

			m_StartWidth = m_Width;
			m_StartHeight = m_Height;
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		if (m_Resizing)
		{
			m_Resizing = false;
			m_Active = true;
			m_OnWindowStateChangedEvent.Broadcast(m_Active);

			if(m_StartHeight != m_Height || m_StartWidth != m_Width)
				m_OnWindowSizeChangedEvent.Broadcast(m_Width, m_Height);
		}
		return 0;

	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}