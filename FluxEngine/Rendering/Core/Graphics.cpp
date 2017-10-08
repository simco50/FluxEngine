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

Graphics::Graphics(HINSTANCE hInstance) :
	m_hInstance(hInstance)
{
}

Graphics::~Graphics()
{
/*
#ifdef _DEBUG
	ComPtr<ID3D11Debug> pDebug;
	HR(m_pDevice->QueryInterface(IID_PPV_ARGS(&pDebug)));
	pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_SUMMARY);
#endif
*/
}

bool Graphics::SetMode(const int width, 
	const int height, 
	const WindowType windowType,
	const bool resizable, 
	const bool vsync, 
	const int multiSample, 
	const int refreshRate)
{
	m_WindowType = windowType;
	m_Resizable = resizable;
	m_Vsync = vsync;
	m_RefreshRate = refreshRate;
	m_Multisample = multiSample;

	if (!RegisterWindowClass())
		return false;
	if (!MakeWindow(width, height))
		return false;

	if (!m_pDevice.IsValid() || m_Multisample != multiSample)
	{
		if (!CreateDevice(width, height))
			return false;
	}
	if (!UpdateSwapchain())
		return false;

	m_pBlendState = make_unique<BlendState>();
	m_pRasterizerState = make_unique<RasterizerState>();
	m_pDepthStencilState = make_unique<DepthStencilState>();

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
	if (pRenderTarget != nullptr)
		m_pDeviceContext->OMSetRenderTargets(1, &pRtv, pRenderTarget->GetDepthStencilView());
}

void Graphics::SetRenderTargets(const vector<RenderTarget*>& pRenderTargets)
{
	vector<ID3D11RenderTargetView*> pRtvs;
	pRtvs.reserve(pRenderTargets.size());
	for (RenderTarget* pRt : pRenderTargets)
		pRtvs.push_back(pRt->GetRenderTargetView());
	m_pDeviceContext->OMSetRenderTargets((UINT)pRenderTargets.size(), pRtvs.data(), pRenderTargets[0]->GetDepthStencilView());
}

void Graphics::SetVertexBuffer(VertexBuffer* pBuffer)
{
	SetVertexBuffers({ pBuffer });
}

void Graphics::SetVertexBuffers(const vector<VertexBuffer*>& pBuffers)
{
	vector<ID3D11Buffer*> buffers;
	vector<unsigned int> strides;
	vector<unsigned int> offsets(pBuffers.size());
	for (const VertexBuffer* pVb : pBuffers)
	{
		buffers.push_back((ID3D11Buffer*)pVb->GetBuffer());
		strides.push_back(pVb->GetStride());
	}

	m_pDeviceContext->IASetVertexBuffers(0, (UINT)pBuffers.size(), buffers.data(), strides.data(), offsets.data());
}

void Graphics::SetIndexBuffer(IndexBuffer* pIndexBuffer)
{
	if (m_pCurrentIndexBuffer != pIndexBuffer)
	{
		m_pDeviceContext->IASetIndexBuffer((ID3D11Buffer*)pIndexBuffer->GetBuffer(), pIndexBuffer->IsSmallStride() ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
		m_pCurrentIndexBuffer = pIndexBuffer;
	}
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

void Graphics::SetInputLayout(InputLayout* pInputLayout)
{
	if (m_pCurrentInputLayout != pInputLayout)
	{
		m_pCurrentInputLayout = pInputLayout;
		m_pDeviceContext->IASetInputLayout((ID3D11InputLayout*)m_pCurrentInputLayout->GetInputLayout());
	}
}

void Graphics::SetViewport(const FloatRect& rect)
{
	D3D11_VIEWPORT viewport;
	viewport.Height = rect.GetHeight() * m_WindowHeight;
	viewport.Width = rect.GetWidth() * m_WindowWidth;
	viewport.TopLeftX = rect.Left * m_WindowWidth;
	viewport.TopLeftY = rect.Top * m_WindowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	m_CurrentViewport = rect;

	m_pDeviceContext->RSSetViewports(1, &viewport);
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

void Graphics::SetTexture(const unsigned int index, Texture* pTexture)
{
	if (index >= m_CurrentSamplerStates.size())
	{
		m_CurrentSamplerStates.resize(index + 1);
		m_CurrentShaderResourceViews.resize(index + 1);
	}

	pTexture->UpdateParameters();

	m_CurrentShaderResourceViews[index] = (ID3D11ShaderResourceView*)pTexture->GetResourceView();
	m_CurrentSamplerStates[index] = (ID3D11SamplerState*)pTexture->GetSamplerState();
}

void Graphics::Draw(const PrimitiveType type, const int vertexStart, const int vertexCount)
{
	SetPrimitiveType(type);
	m_pDeviceContext->Draw(vertexCount, vertexStart);
}

void Graphics::Draw(const PrimitiveType type, const int indexCount, const int indexStart, const int minVertex)
{
	SetPrimitiveType(type);
	m_pDeviceContext->DrawIndexed(indexCount, indexStart, minVertex);
}

void Graphics::Clear(const unsigned int flags, const XMFLOAT4& color, const float depth, const unsigned char stencil)
{
	m_pDefaultRenderTarget->ClearColor(color);
	m_pDefaultRenderTarget->ClearDepth(flags, depth, stencil);
}

void Graphics::PrepareDraw()
{
	if (m_pDepthStencilState->IsDirty())
	{
		ID3D11DepthStencilState* pState = m_pDepthStencilState->Create(m_pDevice.Get());
		m_pDeviceContext->OMSetDepthStencilState(pState, m_pDepthStencilState->GetStencilRef());
	}

	if (m_pRasterizerState->IsDirty())
	{
		ID3D11RasterizerState* pState = m_pRasterizerState->Create(m_pDevice.Get());
		m_pDeviceContext->RSSetState(pState);
	}

	if (m_pBlendState->IsDirty())
	{
		ID3D11BlendState* pBlendState = m_pBlendState->Create(m_pDevice.Get());
		m_pDeviceContext->OMSetBlendState(pBlendState, nullptr, numeric_limits<unsigned int>::max());
	}

	for (unsigned int i = 0; i < m_CurrentSamplerStates.size(); ++i)
	{
		m_pDeviceContext->VSSetSamplers(0, (UINT)m_CurrentSamplerStates.size(), m_CurrentSamplerStates.data());
		m_pDeviceContext->PSSetSamplers(0, (UINT)m_CurrentSamplerStates.size(), m_CurrentSamplerStates.data());
		m_pDeviceContext->VSSetShaderResources(0, (UINT)m_CurrentShaderResourceViews.size(), m_CurrentShaderResourceViews.data());
		m_pDeviceContext->PSSetShaderResources(0, (UINT)m_CurrentShaderResourceViews.size(), m_CurrentShaderResourceViews.data());
	}

	if (m_ScissorRectDirty)
	{
		D3D11_RECT rect = { 
			(LONG)m_CurrentScissorRect.Left, 
			(LONG)m_CurrentScissorRect.Top, 
			(LONG)m_CurrentScissorRect.Right, 
			(LONG)m_CurrentScissorRect.Bottom };
		m_pDeviceContext->RSSetScissorRects(1, &rect);
		m_ScissorRectDirty = false;
	}

	vector<ID3D11Buffer*> pBuffers;
	for (const unique_ptr<ConstantBuffer>& pBuffer : m_pCurrentVertexShader->GetConstantBuffers())
	{
		pBuffers.push_back(pBuffer ? (ID3D11Buffer*)pBuffer->GetBuffer() : nullptr);
		if(pBuffer)
			pBuffer->Apply();
	}
	m_pDeviceContext->VSSetConstantBuffers(0, (UINT)pBuffers.size(), pBuffers.data());

	pBuffers.clear();
	for (const unique_ptr<ConstantBuffer>& pBuffer : m_pCurrentPixelShader->GetConstantBuffers())
	{
		pBuffers.push_back(pBuffer ? (ID3D11Buffer*)pBuffer->GetBuffer() : nullptr);
		if (pBuffer)
			pBuffer->Apply();
	}
	m_pDeviceContext->PSSetConstantBuffers(0, (UINT)pBuffers.size(), pBuffers.data());
}

void Graphics::BeginFrame()
{
	//
}

void Graphics::EndFrame()
{
	//AUTOPROFILE(Present);
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
		FLUX_LOG(ERROR, "[Graphics::SetPrimitiveType()] > Invalid primitive type");
		break;
	}
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

	DWORD windowStyle;
	switch (m_WindowType)
	{
	case WindowType::FULLSCREEN:
		break;
	case WindowType::WINDOWED:
		windowStyle = WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME | WS_CAPTION;
		break;
	case WindowType::BORDERLESS:
		windowWidth = displayWidth;
		windowHeight = displayHeight;
		windowStyle = WS_POPUP;
		break;
	}
	if (m_Resizable)
		windowStyle |= WS_MAXIMIZEBOX;

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

bool Graphics::EnumerateAdapters()
{
	AUTOPROFILE(EnumerateAdapters);

	//Create the factor
	HR(CreateDXGIFactory(IID_PPV_ARGS(m_pFactory.GetAddressOf())));
	vector<IDXGIAdapter*> pAdapters;
	UINT adapterCount = 0;

	int bestAdapterIdx = 0;
	unsigned long bestMemory = 0;

	IDXGIAdapter* pAdapter = nullptr;
	while (m_pFactory->EnumAdapters(adapterCount, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		pAdapter->GetDesc(&desc);

		if (desc.DedicatedVideoMemory > bestMemory)
		{
			bestMemory = (unsigned long)desc.DedicatedVideoMemory;
			bestAdapterIdx = adapterCount;
		}

		wstring gpuDesc(desc.Description);
		FLUX_LOG(INFO, "[%i] %s", adapterCount, string(gpuDesc.begin(), gpuDesc.end()).c_str());

		pAdapters.push_back(pAdapter);
		++adapterCount;
	}
	m_pAdapter = pAdapters[bestAdapterIdx];

	return true;
}

bool Graphics::CreateDevice(const int windowWidth, const int windowHeight)
{
	AUTOPROFILE(CreateDevice);

	EnumerateAdapters();

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
		FLUX_LOG(ERROR, "[Graphics::CreateDevice()] > Feature Level 11_0 not supported!");
		return false;
	}

	if (!CheckMultisampleQuality(DXGI_FORMAT_B8G8R8A8_UNORM, m_Multisample))
		m_Multisample = 1;

	m_pSwapChain.Reset();

	//Create swap chain desctriptor
	DXGI_SWAP_CHAIN_DESC swapDesc;
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.Height = windowHeight;
	swapDesc.BufferDesc.Width = windowWidth;
	swapDesc.BufferDesc.RefreshRate.Denominator = m_RefreshRate;
	swapDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.SampleDesc.Count = m_Multisample;
	swapDesc.SampleDesc.Quality = GetMultisampleQuality(DXGI_FORMAT_R8G8B8A8_UNORM, m_Multisample);
	swapDesc.OutputWindow = m_Hwnd;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = m_WindowType != WindowType::FULLSCREEN;

	//Create the swap chain
	HR(m_pFactory->CreateSwapChain(m_pDevice.Get(), &swapDesc, m_pSwapChain.GetAddressOf()));

	m_WindowWidth = windowWidth;
	m_WindowHeight = windowHeight;

	return true;
}

bool Graphics::UpdateSwapchain()
{
	AUTOPROFILE(UpdateSwapchain);

	assert(m_pDevice.IsValid());
	assert(m_pSwapChain.IsValid());

	m_pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_pDefaultRenderTarget.reset();

	HR(m_pSwapChain->ResizeBuffers(1, m_WindowWidth, m_WindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	ID3D11Texture2D *pBackbuffer = nullptr;
	HR(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackbuffer)));

	RENDER_TARGET_DESC desc = {};
	desc.Width = m_WindowWidth;
	desc.Height = m_WindowHeight;
	desc.pColor = pBackbuffer;
	desc.pDepth = nullptr;
	desc.ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DepthFormat = DXGI_FORMAT_R24G8_TYPELESS;
	desc.MsaaSampleCount = m_Multisample;
	m_pDefaultRenderTarget = unique_ptr<RenderTarget>(new RenderTarget(this));
	m_pDefaultRenderTarget->Create(desc);
	SetRenderTarget(m_pDefaultRenderTarget.get());
	SetViewport(m_CurrentViewport);

	return true;
}

unsigned int Graphics::GetMultisampleQuality(const DXGI_FORMAT format, const unsigned int sampleCount) const
{
	if (sampleCount < 2)
		return 0; // Not multisampled, should use quality 0

	if (m_pDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_10_1)
		return 0xffffffff; // D3D10.1+ standard level

	UINT numLevels = 0;
	HRESULT hr = m_pDevice->CheckMultisampleQualityLevels(format, sampleCount, &numLevels);
	if (hr != S_OK || !numLevels)
		return 0; // Errored or sample count not supported
	else
		return numLevels - 1; // D3D10.0 and below: use the best quality
}

bool Graphics::CheckMultisampleQuality(const DXGI_FORMAT format, const unsigned int sampleCount) const
{
	if (sampleCount < 2)
		return true; // Not multisampled, should use quality 0

	UINT numLevels = 0;
	HRESULT hr = m_pDevice->CheckMultisampleQualityLevels(format, sampleCount, &numLevels);
	if (hr != S_OK)
		return false; // Errored or sample count not supported
	else
		return numLevels > 0; // D3D10.0 and below: use the best quality
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
		m_WindowWidth = LOWORD(lParam);
		m_WindowHeight = HIWORD(lParam);
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
				UpdateSwapchain();
			}
			else if (wParam == SIZE_RESTORED)
			{
				// Restoring from minimized state?
				if (m_Minimized)
				{
					OnPause(false);
					m_Minimized = false;
					UpdateSwapchain();
				}
				// Restoring from maximized state?
				else if (m_Maximized)
				{
					OnPause(false);
					m_Maximized = false;
					UpdateSwapchain();
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
		OnPause(true);
		m_Resizing = true;
		return 0;

	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		OnPause(false);
		m_Resizing = false;
		UpdateSwapchain();
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