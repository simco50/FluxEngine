#include "stdafx.h"
#include "Renderer.h"
#include "RenderItem.h"
#include "Materials/Material.h"
#include "Graphics/MeshFilter.h"
#include "Graphics/RenderTarget.h"

Renderer::~Renderer()
{
}

HRESULT Renderer::Initialize(const HWND& hwnd, const GameSettings& gameSettings)
{
	m_pGameSettings = &gameSettings;
	m_hwnd = hwnd;

	EnumerateAdapters();
	CreateDevice();
	CreateSwapchain();
	OnResize();

	return 0;
}

HRESULT Renderer::Shutdown()
{
	m_pSwapChain->SetFullscreenState(false, nullptr);
	return 0;
}

void Renderer::Submit(const RenderItem* item)
{
	m_RenderQueue.push_back(item);
}

void Renderer::Render()
{
	m_pDefaultRenderTarget->ClearColor();
	m_pDefaultRenderTarget->ClearDepth();

	SortQueue();

	for (const RenderItem* item : m_RenderQueue)
	{
		RenderSingle(item);
	}

	FlushQueue();
}


void Renderer::RenderSingle(const RenderItem* item)
{
	//Render a single RenderItem

	m_pD3DeviceContext->IASetInputLayout(item->Material->GetInputLayout());
	m_pD3DeviceContext->IASetPrimitiveTopology(item->Topology);
	m_pD3DeviceContext->IASetIndexBuffer(item->Mesh->GetIndexBuffer(), DXGI_FORMAT_R8_UINT, 0);
	UINT strides = item->Material->GetInputLayoutDesc()->VertexStride;
	UINT offset = 0;

	ID3D11Buffer* vertexBuffer = item->Mesh->GetVertexBuffer();
	m_pD3DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	item->Material->GetTechnique()->GetDesc(&techDesc);

	for (size_t i = 0; i < techDesc.Passes; i++)
	{
		item->Material->GetTechnique()->GetPassByIndex(i)->Apply(0, m_pD3DeviceContext);
		m_pD3DeviceContext->DrawIndexed(item->Mesh->IndexCount(), 0, 0);
	}
}

void Renderer::FlushQueue()
{
	//Clear the queue and prepare it for the next frame
	m_RenderQueue.clear();
}

void Renderer::SortQueue()
{
	//Sort the items according to the material specifications
}


#pragma region INITIALIZATION

HRESULT Renderer::EnumerateAdapters()
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

HRESULT Renderer::CreateDevice()
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

	m_pD3Device = m_pDevice.Get();
	m_pD3DeviceContext = m_pDeviceContext.Get();

	//Check for 4X MSAA Quality support
	UINT numQualityLevels = 0;
	m_pDevice->CheckMultisampleQualityLevels(m_BackBufferFormat, 4, &numQualityLevels);
	assert(m_pGameSettings->MsaaQuality > 0);

	return S_OK;
}

HRESULT Renderer::CreateSwapchain()
{
	m_pSwapChain.Reset();

	//Create swap chain desctriptor
	DXGI_SWAP_CHAIN_DESC swapDesc;
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Format = m_BackBufferFormat;
	swapDesc.BufferDesc.Height = m_pGameSettings->Height;
	swapDesc.BufferDesc.Width = m_pGameSettings->Width;
	swapDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	if (m_pGameSettings->MSAA)
	{
		swapDesc.SampleDesc.Count = 4;
		swapDesc.SampleDesc.Quality = m_pGameSettings->MsaaQuality - 1;
	}
	else
	{
		swapDesc.SampleDesc.Count = 1;
		swapDesc.SampleDesc.Quality = 0;
	}
	swapDesc.OutputWindow = m_hwnd;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = m_pGameSettings->WindowStyle == WindowStyle::FULLSCREEN ? false : true;

	//Create the swap chain
	HR(m_pFactory->CreateSwapChain(m_pDevice.Get(), &swapDesc, m_pSwapChain.GetAddressOf()));

	return S_OK;
}

void Renderer::OnResize()
{
	assert(m_pDevice);
	assert(m_pSwapChain);

	m_pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_pDefaultRenderTarget.reset();

	HR(m_pSwapChain->ResizeBuffers(1, m_pGameSettings->Width, m_pGameSettings->Height, m_BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	ID3D11Texture2D *pBackbuffer = nullptr;
	HR(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackbuffer)));

	RENDER_TARGET_DESC desc;
	desc.Width = m_pGameSettings->Width;
	desc.Height = m_pGameSettings->Height;
	desc.pColor = pBackbuffer;
	desc.ColorFormat = m_BackBufferFormat;
	desc.DepthFormat = m_DepthStencilFormat;
	desc.MSAA = m_pGameSettings->MSAA;
	desc.DepthSRV = true;

	m_pDefaultRenderTarget = make_unique<RenderTarget>(m_pDevice.Get(), m_pDeviceContext.Get());
	m_pDefaultRenderTarget->Create(desc);
	ID3D11RenderTargetView* rtv = m_pDefaultRenderTarget->GetRenderTargetView();

	//Bind views to the output merger state
	m_pDeviceContext->OMSetRenderTargets(1, &rtv, m_pDefaultRenderTarget->GetDepthStencilView());

	m_Viewport.Height = (float)m_pGameSettings->Height;
	m_Viewport.Width = (float)m_pGameSettings->Width;
	m_Viewport.MaxDepth = 1.0f;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_pDeviceContext->RSSetViewports(1, &m_Viewport);
}

#pragma endregion INITIALIZATION