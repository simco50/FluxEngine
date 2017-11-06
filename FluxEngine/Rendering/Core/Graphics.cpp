#include "stdafx.h"
#include "Graphics.h"
#include "RenderTarget.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ShaderVariation.h"
#include "InputLayout.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "BlendState.h"
#include "Shader.h"



#ifdef D3D11
#include "D3D11/D3D11Graphics.hpp"
#include "D3D11/D3D11GraphicsImpl.h"
#endif
#include "UI/ImmediateUI.h"

Graphics::Graphics(HINSTANCE hInstance) :
	m_hInstance(hInstance)
{
	m_pImpl = make_unique<GraphicsImpl>();
}

void Graphics::SetWindowTitle(const string& title)
{
	SetWindowText(m_Hwnd, title.c_str());
}

void Graphics::SetWindowPosition(const XMFLOAT2& position)
{
	SetWindowPos(m_Hwnd, HWND_TOP, (int)position.x, (int)position.y, -1, -1, SWP_NOSIZE);
}

void Graphics::SetScissorRect(const bool enabled, const IntRect& rect)
{
	m_pRasterizerState->SetScissorEnabled(enabled);

	if (enabled && rect != m_CurrentScissorRect)
	{
		m_CurrentScissorRect = rect;
		m_ScissorRectDirty = true;
	}
}

Shader* Graphics::GetShader(const string filePath)
{
	string shaderName = Paths::GetFileNameWithoutExtension(filePath);
	auto pIt = m_Shaders.find(shaderName);
	if (pIt != m_Shaders.end())
		return pIt->second.get();

	unique_ptr<Shader> pShader = make_unique<Shader>(this);
	if (!pShader->Load(filePath))
		return nullptr;

	m_Shaders[shaderName] = std::move(pShader);
	return m_Shaders[shaderName].get();
}

void Graphics::OnPause(const bool paused)
{
	m_Paused = paused;
	if (paused)
		GameTimer::Stop();
	else
		GameTimer::Start();
}

void Graphics::GetDebugInfo(unsigned int& batchCount, unsigned int& primitiveCount)
{
	batchCount = m_BatchCount;
	primitiveCount = m_PrimitiveCount;
}

bool Graphics::RegisterWindowClass()
{
	AUTOPROFILE(RegisterWindowClass);

	WNDCLASSA wc;

	wc.hInstance = GetModuleHandle(0);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpfnWndProc = WndProcStatic;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = "wndClass";
	wc.lpszMenuName = nullptr;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	if (!RegisterClassA(&wc))
	{
		auto error = GetLastError();
		FLUX_LOG_HR("[Graphics::RegisterWindowClass()]", HRESULT_FROM_WIN32(error));
		return false;
	}
	return true;
}

bool Graphics::MakeWindow(int windowWidth, int windowHeight)
{
	AUTOPROFILE(MakeWindow);

	int displayWidth = GetSystemMetrics(SM_CXSCREEN);
	int displayHeight = GetSystemMetrics(SM_CYSCREEN);

	DWORD windowStyle = WS_VISIBLE;
	switch (m_WindowType)
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

	m_Hwnd = CreateWindowA(
		"wndClass",
		m_WindowTitle.c_str(),
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

LRESULT CALLBACK Graphics::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Graphics* pThis = nullptr;

	if (message == WM_NCCREATE)
	{
		pThis = static_cast<Graphics*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		SetLastError(0);
		if (!SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis)))
		{
			if (GetLastError() != 0)
				return 0;
		}
	}
	else
	{
		pThis = reinterpret_cast<Graphics*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}
	if (pThis)
	{
		LRESULT callback = pThis->WndProc(hWnd, message, wParam, lParam);
		return callback;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT Graphics::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImmediateUI::WndProc(message, wParam, lParam))
		return 0;

	switch (message)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		OnPause(LOWORD(wParam) == WA_INACTIVE || m_Minimized);
		return 0;

		// WM_SIZE is sent when the user resizes the window.
	case WM_SIZE:
	{
		// Save the new client area dimensions.
		m_WindowWidth = LOWORD(lParam);
		m_WindowHeight = HIWORD(lParam);
		if (m_pImpl->m_pDevice.IsValid())
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_Minimized = true;
				m_Maximized = false;
				OnPause(true);
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				UpdateSwapchain();
				m_Maximized = true;
				m_Minimized = false;
				OnPause(false);
			}
			else if (wParam == SIZE_RESTORED)
			{
				// Restoring from minimized state?
				if (m_Minimized)
				{
					UpdateSwapchain();
					m_Minimized = false;
					OnPause(false);
				}
				// Restoring from maximized state?
				else if (m_Maximized)
				{
					UpdateSwapchain();
					m_Maximized = false;
					OnPause(false);
				}
				else if (!m_Resizing) // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					UpdateSwapchain();
				}
			}
		}
		return 0;
	}

	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		m_Resizing = true;
		OnPause(true);
		return 0;

	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		UpdateSwapchain();
		m_Resizing = false;
		OnPause(false);
		return 0;

	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		if (wParam == 'P')
			TakeScreenshot();

	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}