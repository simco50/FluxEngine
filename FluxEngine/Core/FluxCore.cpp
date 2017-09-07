#include "stdafx.h"
#include "FluxCore.h"
#include "../resource.h"
#include "../Managers/AudioEngine.h"
#include "../Graphics/RenderTarget.h"
#include "../UI/ImgUIDrawer.h"
#include "../Physics/PhysX/PhysicsCore.h"

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
	CleanupD3D();
	AudioEngine::DestroyInstance();
	ResourceManager::Release();
	Console::Release();

	InputEngine::GetInstance()->DestroyInstance();

	m_pUIDrawer->Shutdown();
	SafeDelete(m_pUIDrawer);
}

void FluxCore::CleanupD3D()
{
	m_pSwapChain->SetFullscreenState(false, nullptr);
}

int FluxCore::Run(HINSTANCE hInstance)
{
	Console::Initialize();

	PrepareGame();

	m_hInstance = hInstance;

	HR(RegisterWindowClass());
	HR(MakeWindow());
	HR(EnumAdapters());
	HR(InitializeD3D());
	OnResize();

	InitializeHighDefinitionMouse();

	ResourceManager::Initialize(m_pDevice.Get());
	Initialize(&m_EngineContext);

	GameTimer::Reset();

	m_pUIDrawer = new ImgUIDrawer();
	m_pUIDrawer->Initialize(&m_EngineContext);

	InputEngine::GetInstance()->Initialize();

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
	m_pDefaultRenderTarget->ClearColor();
	m_pDefaultRenderTarget->ClearDepth();

	CalculateFrameStats();

	AudioEngine::GetInstance()->Update();
	InputEngine::GetInstance()->Update();

	Update();

	m_pUIDrawer->NewFrame();

	//Render the game
	Render();

	m_pUIDrawer->Render();


	m_pSwapChain->Present(m_EngineContext.GameSettings.VerticalSync ? 1 : 0, 0);
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

HRESULT FluxCore::EnumAdapters()
{
	//Create the factor
	HR(CreateDXGIFactory(IID_PPV_ARGS(m_pFactory.GetAddressOf())));
	vector<IDXGIAdapter*> pAdapters;
	UINT adapterCount = 0;

	IDXGIAdapter* pAdapter = nullptr;
	while (m_pFactory->EnumAdapters(adapterCount, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		pAdapters.push_back(pAdapter);
		++adapterCount;
	}
	m_pAdapter = pAdapters[0];
	return S_OK;
}

HRESULT FluxCore::InitializeD3D()
{
	//Create the device
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	HR(D3D11CreateDevice(
		m_pAdapter.Get(),
		D3D_DRIVER_TYPE_UNKNOWN,
		nullptr,
		createDeviceFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		m_pDevice.GetAddressOf(),
		&featureLevel,
		m_pDeviceContext.GetAddressOf())
	);

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		Console::Log("Feature Level 11_0 not supported!", LogType::ERROR);
		return E_FAIL;
	}

	m_EngineContext.D3Device = m_pDevice.Get();
	m_EngineContext.D3DeviceContext = m_pDeviceContext.Get();

	//Check for 4X MSAA Quality support
	m_pDevice->CheckMultisampleQualityLevels(m_BackBufferFormat, 4, &m_EngineContext.GameSettings.MsaaQuality);
	assert(m_EngineContext.GameSettings.MsaaQuality > 0);

	CreateSwapChain();

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

void FluxCore::CreateSwapChain()
{
	m_pSwapChain.Reset();

	//Create swap chain desctriptor
	DXGI_SWAP_CHAIN_DESC swapDesc;
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Format = m_BackBufferFormat;
	swapDesc.BufferDesc.Height = m_EngineContext.GameSettings.Height;
	swapDesc.BufferDesc.Width = m_EngineContext.GameSettings.Width;
	swapDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	if (m_EngineContext.GameSettings.MSAA && m_EngineContext.GameSettings.UseDeferredRendering == false)
	{
		swapDesc.SampleDesc.Count = 4;
		swapDesc.SampleDesc.Quality = m_EngineContext.GameSettings.MsaaQuality - 1;
	}
	else
	{
		swapDesc.SampleDesc.Count = 1;
		swapDesc.SampleDesc.Quality = 0;
	}
	swapDesc.OutputWindow = m_EngineContext.Hwnd;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = m_EngineContext.GameSettings.WindowStyle == WindowStyle::FULLSCREEN ? false : true;

	//Create the swap chain
	HR(m_pFactory->CreateSwapChain(m_pDevice.Get(), &swapDesc, m_pSwapChain.GetAddressOf()));
}

void FluxCore::OnResize()
{
	assert(m_pDevice);
	assert(m_pSwapChain);
	
	m_pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_pDefaultRenderTarget.reset();

	HR(m_pSwapChain->ResizeBuffers(1, m_EngineContext.GameSettings.Width, m_EngineContext.GameSettings.Height, m_BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	ID3D11Texture2D *pBackbuffer = nullptr;
	HR(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackbuffer)));

	RENDER_TARGET_DESC desc;
	desc.Width = m_EngineContext.GameSettings.Width;
	desc.Height = m_EngineContext.GameSettings.Height;
	desc.pColor = pBackbuffer;
	desc.ColorFormat = m_BackBufferFormat;
	desc.DepthFormat = m_DepthStencilFormat;
	desc.MSAA = m_EngineContext.GameSettings.UseDeferredRendering ? false : m_EngineContext.GameSettings.MSAA;
	desc.DepthSRV = true;

	m_pDefaultRenderTarget = make_unique<RenderTarget>(&m_EngineContext);
	m_pDefaultRenderTarget->Create(&desc);
	m_EngineContext.DefaultRenderTarget = m_pDefaultRenderTarget.get();
	ID3D11RenderTargetView* rtv = m_pDefaultRenderTarget->GetRenderTargetView();

	//Bind views to the output merger state
	m_pDeviceContext->OMSetRenderTargets(1, &rtv, m_pDefaultRenderTarget->GetDepthStencilView());

	m_Viewport.Height = m_EngineContext.GameSettings.Height;
	m_Viewport.Width = m_EngineContext.GameSettings.Width;
	m_Viewport.MaxDepth = 1.0f;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_pDeviceContext->RSSetViewports(1, &m_Viewport);
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
		if (m_pDevice)
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
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				// Restoring from minimized state?
				if (m_Minimized)
				{
					OnPause(false);
					m_Minimized = false;
					OnResize();
				}
				// Restoring from maximized state?
				else if (m_Maximized)
				{
					OnPause(false);
					m_Maximized = false;
					OnResize();
				}
				else if (!m_Resizing) // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
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
		OnResize();
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
			InputEngine::GetInstance()->SetMouseMovement(XMFLOAT2(xPosRelative, yPosRelative));
		}
		return 0;
	}

	default:
		break;
	}

	m_pUIDrawer->WndProc(message, wParam, lParam);

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