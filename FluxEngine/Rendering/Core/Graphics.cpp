#include "stdafx.h"
#include "Graphics.h"
#include "RenderTarget.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "..\ShaderVariation.h"


Graphics::Graphics()
{
}

Graphics::~Graphics()
{
}

bool Graphics::SetMode(const int width, 
	const int height, 
	const bool fullscreen, 
	const bool borderless, 
	const bool resizable, 
	const bool vsync, 
	const int multiSample, 
	const int monitor, 
	const int refreshRate)
{
	m_Fullscreen = fullscreen;
	m_Borderless = borderless;
	m_Resizable = resizable;
	m_Vsync = vsync;
	m_Monitor = monitor;
	m_RefreshRate = refreshRate;

	if (!m_Hwnd)
	{
		if (!RegisterWindowClass())
			return false;
		if (!MakeWindow())
			return false;
	}

	if (!m_pDevice.IsValid() || m_Multisample != multiSample)
	{
		if (!CreateDevice(width, height, multiSample))
			return false;
	}
	if (!UpdateSwapchain(width, height))
		return false;

	m_WindowWidth = width;
	m_WindowHeight = height;
	m_Multisample = multiSample;

	Clear(0);
	m_pSwapChain->Present(0, 0);

	return true;
}

void Graphics::SetWindowTitle(const string& title)
{
	SetWindowText(m_Hwnd, title.c_str());
}

void Graphics::SetWindowPosition(const XMFLOAT2& position)
{
	SetWindowPos(m_Hwnd, HWND_TOP, (int)position.x, (int)position.y, -1, -1, SWP_NOSIZE);
}

void Graphics::SetRenderTarget(RenderTarget* pRenderTarget)
{
	ID3D11RenderTargetView* pRtv = pRenderTarget->GetRenderTargetView();
	if (pRenderTarget == nullptr)
		m_pDeviceContext->OMSetRenderTargets(1, &pRtv, pRenderTarget->GetDepthStencilView());
}

void Graphics::SetRenderTargets(const vector<RenderTarget*>& pRenderTargets)
{
	vector<ID3D11RenderTargetView*> pRtvs;
	pRtvs.reserve(pRenderTargets.size());
	for (RenderTarget* pRt : pRenderTargets)
		pRtvs.push_back(pRt->GetRenderTargetView());
	m_pDeviceContext->OMSetRenderTargets(pRenderTargets.size(), pRtvs.data(), pRenderTargets[0]->GetDepthStencilView());
}

void Graphics::SetVertexBuffer(VertexBuffer* pBuffer)
{
	if (m_pCurrentVertexBuffer == pBuffer)
		return;
	ID3D11Buffer* pNativeBuffer = pBuffer->GetBuffer();
	const unsigned int vertexStride = pBuffer->GetVertexStride();
	m_pDeviceContext->IASetVertexBuffers(0, 1, &pNativeBuffer, &vertexStride, 0);
}

void Graphics::SetVertexBuffers(const vector<VertexBuffer*>& pBuffers)
{
	vector<ID3D11Buffer*> buffers;
	vector<unsigned int> strides;
	for (const VertexBuffer* pVb : pBuffers)
	{
		buffers.push_back(pVb->GetBuffer());
		strides.push_back(pVb->GetVertexStride());
	}

	m_pDeviceContext->IASetVertexBuffers(0, pBuffers.size(), buffers.data(), strides.data(), 0);
}

void Graphics::SetIndexBuffer(IndexBuffer* pIndexBuffer)
{
	m_pDeviceContext->IASetIndexBuffer(pIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
}

void Graphics::SetShaders(ShaderVariation* pVertexShader, ShaderVariation* pPixelShader)
{
	if (pVertexShader != m_pCurrentVertexShader)
	{
		m_pDeviceContext->VSSetShader((ID3D11VertexShader*)pVertexShader->GetShaderObject(), nullptr, 0);
		m_pCurrentVertexShader = pVertexShader;
	}
	if (pPixelShader != m_pCurrentVertexShader)
	{
		m_pDeviceContext->PSSetShader((ID3D11PixelShader*)pPixelShader->GetShaderObject(), nullptr, 0);
		m_pCurrentPixelShader = pPixelShader;
	}
}

void Graphics::Draw(const PrimitiveType type, const int vertexStart, const int vertexCount)
{
	UNREFERENCED_PARAMETER(vertexStart);

	SetPrimitiveType(type);
	m_pDeviceContext->Draw(vertexCount, vertexCount);
}

void Graphics::Draw(const PrimitiveType type, const int indexStart, const int indexCount, const int minVertex, const int vertexCount)
{
	UNREFERENCED_PARAMETER(vertexCount);

	SetPrimitiveType(type);
	m_pDeviceContext->DrawIndexed(indexCount, indexStart, minVertex);
}

void Graphics::Clear(const unsigned int flags, const XMFLOAT4& color, const float depth, const unsigned int stencil)
{
	UNREFERENCED_PARAMETER(stencil);
	UNREFERENCED_PARAMETER(depth);
	UNREFERENCED_PARAMETER(color);
	UNREFERENCED_PARAMETER(flags);

	m_pCurrentRenderTarget->ClearColor();
	m_pCurrentRenderTarget->ClearDepth();
}

void Graphics::BeginFrame()
{
	//
}

void Graphics::EndFrame()
{
	AUTOPROFILE(Present);
	m_pSwapChain->Present(m_Vsync ? 1 : 0, 0);
}

void Graphics::OnPause(const bool paused)
{
	m_Paused = paused;
	if (paused)
		GameTimer::Stop();
	else
		GameTimer::Start();
}

void Graphics::SetPrimitiveType(const PrimitiveType type)
{
	if (m_CurrentPrimitiveType == type)
		return;
	m_CurrentPrimitiveType = type;
	switch (type)
	{
	case PrimitiveType::TRIANGLELIST:
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;
	case PrimitiveType::POINTLIST:
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		break;
	case PrimitiveType::TRIANGLESTRIP:
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		break;
	case PrimitiveType::UNDEFINED:
	default:
		Console::Log("Invalid primitive type", LogType::ERROR);
		break;
	}
}

bool Graphics::RegisterWindowClass()
{
	WNDCLASSA wc;

	wc.hInstance = m_hInstance;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hCursor = 0;
	wc.hIcon = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpfnWndProc = WndProcStatic;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = "wndClass";
	wc.lpszMenuName = nullptr;

	if (!RegisterClassA(&wc))
	{
		auto error = GetLastError();
		return HRESULT_FROM_WIN32(error);
	}
	return S_OK;
}

bool Graphics::MakeWindow()
{
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (m_Borderless)
	{
		m_WindowWidth = screenWidth;
		m_WindowHeight = screenHeight;
		windowStyle = WS_POPUP;
	}

	RECT windowRect = { 0, 0, m_WindowWidth, m_WindowHeight };
	AdjustWindowRect(&windowRect, windowStyle, false);

	int x = (screenWidth - windowRect.right) / 2;
	int y = (screenHeight - windowRect.bottom) / 2;

	m_Hwnd = CreateWindowA(
		"wndClass",
		m_WindowTitle.c_str(),
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

	if (m_Hwnd == nullptr)
	{
		auto error = GetLastError();
		return HRESULT_FROM_WIN32(error);
	}

	ShowWindow(m_Hwnd, SW_SHOW);
	UpdateWindow(m_Hwnd);

	return S_OK;
}

bool Graphics::EnumerateAdapters()
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

	return true;
}

bool Graphics::CreateDevice(const int windowWidth, const int windowHeight, const int multiSample)
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
		return false;
	}

	m_pSwapChain.Reset();

	//Create swap chain desctriptor
	DXGI_SWAP_CHAIN_DESC swapDesc;
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	swapDesc.BufferDesc.Height = windowWidth;
	swapDesc.BufferDesc.Width = windowHeight;
	swapDesc.BufferDesc.RefreshRate.Denominator = m_RefreshRate;
	swapDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	if (multiSample > 0)
	{
		swapDesc.SampleDesc.Count = multiSample;
		swapDesc.SampleDesc.Quality = m_MsaaQuality - 1;
	}
	else
	{
		swapDesc.SampleDesc.Count = 1;
		swapDesc.SampleDesc.Quality = 0;
	}
	swapDesc.OutputWindow = m_Hwnd;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = !m_Fullscreen;

	//Create the swap chain
	HR(m_pFactory->CreateSwapChain(m_pDevice.Get(), &swapDesc, m_pSwapChain.GetAddressOf()));

	return true;
}

bool Graphics::UpdateSwapchain(const int windowWidth, const int windowHeight)
{
	assert(m_pDevice.IsValid());
	assert(m_pSwapChain.IsValid());

	m_pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	SafeDelete(m_pCurrentRenderTarget);

	HR(m_pSwapChain->ResizeBuffers(1, windowWidth, windowHeight, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	ID3D11Texture2D *pBackbuffer = nullptr;
	HR(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackbuffer)));

	RENDER_TARGET_DESC desc;
	desc.Width = windowWidth;
	desc.Height = windowHeight;
	desc.pColor = pBackbuffer;
	desc.ColorFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//desc.DepthFormat = m_DepthStencilFormat;
	desc.MSAA = m_Multisample > 0;
	desc.MsaaQuality = m_MsaaQuality;
	desc.DepthSRV = true;

	m_pCurrentRenderTarget = new RenderTarget(m_pDevice.Get(), m_pDeviceContext.Get());
	m_pCurrentRenderTarget->Create(desc);
	ID3D11RenderTargetView* rtv = m_pCurrentRenderTarget->GetRenderTargetView();

	//Bind views to the output merger state
	m_pDeviceContext->OMSetRenderTargets(1, &rtv, m_pCurrentRenderTarget->GetDepthStencilView());

	m_Viewport.Height = (float)windowHeight;
	m_Viewport.Width = (float)windowWidth;
	m_Viewport.MaxDepth = 1.0f;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_pDeviceContext->RSSetViewports(1, &m_Viewport);

	m_WindowWidth = windowWidth;
	m_WindowHeight = windowHeight;

	return true;
}

LRESULT CALLBACK Graphics::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CREATE)
	{
		CREATESTRUCT *pCS = reinterpret_cast<CREATESTRUCT*>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG>(pCS->lpCreateParams));
	}
	else
	{
		Graphics* pThisGame = reinterpret_cast<Graphics*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
		if (pThisGame) return pThisGame->WndProc(hWnd, message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT Graphics::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
		if (m_pDevice.IsValid())
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
				UpdateSwapchain(newWidth, newHeight);
			}
			else if (wParam == SIZE_RESTORED)
			{
				// Restoring from minimized state?
				if (m_Minimized)
				{
					OnPause(false);
					m_Minimized = false;
					UpdateSwapchain(newWidth, newHeight);
				}
				// Restoring from maximized state?
				else if (m_Maximized)
				{
					OnPause(false);
					m_Maximized = false;
					UpdateSwapchain(newWidth, newHeight);
				}
				else if (!m_Resizing) // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					UpdateSwapchain(newWidth, newHeight);
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
		UpdateSwapchain(m_WindowWidth, m_WindowHeight);
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