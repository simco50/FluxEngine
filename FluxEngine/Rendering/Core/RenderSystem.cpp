#include "stdafx.h"
#include "RenderSystem.h"
#include "RenderItem.h"
#include "Materials/Material.h"
#include "Rendering/MeshFilter.h"
#include "RenderTarget.h"
#include "UI/ImgUIDrawer.h"

HRESULT RenderSystem::Initialize(const RendererDesc& desc)
{
	m_RendererDesc = desc;

	EnumerateAdapters();
	CreateDevice();
	CreateSwapchain();
	OnResize();

	return S_OK;
}

HRESULT RenderSystem::Shutdown()
{
	SafeDelete(m_pDefaultRenderTarget);
	return 0;
}

void RenderSystem::NewFrame()
{
	m_pDefaultRenderTarget->ClearColor();
	m_pDefaultRenderTarget->ClearDepth();
}

void RenderSystem::Submit(const RenderItem& item)
{
	m_RenderQueue.push_back(item);
}

void RenderSystem::Render()
{
	SortQueue();

	for (const RenderItem& item : m_RenderQueue)
	{
		RenderSingle(item);
	}

	FlushQueue();
}

void RenderSystem::RenderImmediate(const RenderItem& item)
{
	RenderSingle(item);
}

void RenderSystem::RenderSingle(const RenderItem& item)
{
	item.Material->Update(item.WorldMatrix);

	SetInputLayout(item.Material->GetInputLayout());
	SetPrimitiveTopology(item.Topology);

	SetIndexBuffer(item.IndexBuffer);
	UINT strides = item.Material->GetInputLayoutDesc()->VertexStride;
	UINT offset = 0;
	SetVertexBuffers(0, item.VertexBuffers, &strides, &offset);
	SetRenderTarget(item.RenderTarget);

	D3DX11_TECHNIQUE_DESC techDesc;
	item.Material->GetTechnique()->GetDesc(&techDesc);

	for (size_t i = 0; i < techDesc.Passes; i++)
	{
		item.Material->GetTechnique()->GetPassByIndex(i)->Apply(0, m_pDeviceContext.Get());
		if (item.IndexBuffer == nullptr)
			m_pDeviceContext->Draw(item.VertexCount, 0);
		else
			m_pDeviceContext->DrawIndexed(item.IndexCount, 0, 0);
	}
}

void RenderSystem::FlushQueue()
{
	//Clear the queue and prepare it for the next frame
	m_RenderQueue.clear();
}

void RenderSystem::SortQueue()
{
	//Sort the items according to the renderqueue

	std::sort(m_RenderQueue.begin(), m_RenderQueue.end(), 
		[](const RenderItem& a, const RenderItem& b) 
	{
		if (a.Material->GetDesc().RenderQueue == b.Material->GetDesc().RenderQueue)
			return a.Material < b.Material;
		return a.Material->GetDesc().RenderQueue < b.Material->GetDesc().RenderQueue;
	});
}


#pragma region INITIALIZATION

HRESULT RenderSystem::EnumerateAdapters()
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

HRESULT RenderSystem::CreateDevice()
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
		FLUX_LOG(ERROR, "Feature Level 11_0 not supported!");
		return E_FAIL;
	}

	//Check for 4X MSAA Quality support
	m_pDevice->CheckMultisampleQualityLevels(m_BackBufferFormat, 4, &m_RendererDesc.MsaaQuality);
	assert(m_RendererDesc.MsaaQuality > 0);

	return S_OK;
}

void RenderSystem::SetRenderTargets(const vector<RenderTarget*>& renderTargets)
{
	assert(renderTargets.size() > 0);

	vector<ID3D11RenderTargetView*> rtvs;
	for (const RenderTarget* rtv : renderTargets)
		rtvs.push_back(rtv->GetRenderTargetView());
	m_pDeviceContext->OMSetRenderTargets(renderTargets.size(), rtvs.data(), renderTargets[0]->GetDepthStencilView());
}

void RenderSystem::SetRenderTarget(RenderTarget* pRenderTarget)
{
	if (pRenderTarget == nullptr)
	{
		ID3D11RenderTargetView* pRtv = m_pDefaultRenderTarget->GetRenderTargetView();
		m_pDeviceContext->OMSetRenderTargets(1, &pRtv, m_pDefaultRenderTarget->GetDepthStencilView());
	}
	else
	{
		ID3D11RenderTargetView* pRtv = pRenderTarget->GetRenderTargetView();
		m_pDeviceContext->OMSetRenderTargets(1, &pRtv, pRenderTarget->GetDepthStencilView());
	}
}

HRESULT RenderSystem::CreateSwapchain()
{
	m_pSwapChain.Reset();

	//Create swap chain desctriptor
	DXGI_SWAP_CHAIN_DESC swapDesc;
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Format = m_BackBufferFormat;
	swapDesc.BufferDesc.Height = m_RendererDesc.WindowHeight;
	swapDesc.BufferDesc.Width = m_RendererDesc.WindowWidth;
	swapDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	if (m_RendererDesc.Msaa)
	{
		swapDesc.SampleDesc.Count = 4;
		swapDesc.SampleDesc.Quality = m_RendererDesc.MsaaQuality - 1;
	}
	else
	{
		swapDesc.SampleDesc.Count = 1;
		swapDesc.SampleDesc.Quality = 0;
	}
	swapDesc.OutputWindow = m_RendererDesc.Hwnd;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = m_RendererDesc.WindowStyle == WindowStyle::FULLSCREEN ? false : true;

	//Create the swap chain
	HR(m_pFactory->CreateSwapChain(m_pDevice.Get(), &swapDesc, m_pSwapChain.GetAddressOf()));

	return S_OK;
}

void RenderSystem::OnResize()
{
	assert(m_pDevice);
	assert(m_pSwapChain);

	m_pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	SafeDelete(m_pDefaultRenderTarget);

	HR(m_pSwapChain->ResizeBuffers(1, m_RendererDesc.WindowWidth, m_RendererDesc.WindowHeight, m_BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	ID3D11Texture2D *pBackbuffer = nullptr;
	HR(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackbuffer)));

	RENDER_TARGET_DESC desc;
	desc.Width = m_RendererDesc.WindowWidth;
	desc.Height = m_RendererDesc.WindowHeight;
	desc.pColor = pBackbuffer;
	desc.ColorFormat = m_BackBufferFormat;
	desc.DepthFormat = m_DepthStencilFormat;
	desc.MsaaSampleCount = m_RendererDesc.Msaa;
	desc.MsaaQuality = m_RendererDesc.MsaaQuality;
	desc.DepthSRV = true;

	m_pDefaultRenderTarget = new RenderTarget(m_pDevice.Get(), m_pDeviceContext.Get());
	m_pDefaultRenderTarget->Create(desc);
	ID3D11RenderTargetView* rtv = m_pDefaultRenderTarget->GetRenderTargetView();

	//Bind views to the output merger state
	m_pDeviceContext->OMSetRenderTargets(1, &rtv, m_pDefaultRenderTarget->GetDepthStencilView());

	m_Viewport.Height = (float)m_RendererDesc.WindowHeight;
	m_Viewport.Width = (float)m_RendererDesc.WindowWidth;
	m_Viewport.MaxDepth = 1.0f;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_pDeviceContext->RSSetViewports(1, &m_Viewport);
}

void RenderSystem::Present()
{
	m_pSwapChain->Present(m_RendererDesc.VerticalSync ? 1 : 0, 0);
}

void RenderSystem::SetInputLayout(ID3D11InputLayout* pInputLayout)
{
	if (pInputLayout != m_CurrentRenderState.InputLayout)
	{
		m_pDeviceContext->IASetInputLayout(pInputLayout);
		m_CurrentRenderState.InputLayout = pInputLayout;
	}
}

void RenderSystem::SetPrimitiveTopology(const D3D11_PRIMITIVE_TOPOLOGY& topology)
{
	if (topology != m_CurrentRenderState.PrimitiveTopology)
	{
		m_pDeviceContext->IASetPrimitiveTopology(topology);
		m_CurrentRenderState.PrimitiveTopology = topology;
	}
}

void RenderSystem::SetVertexBuffer(const int startSlot, ID3D11Buffer* pVertexBuffer, UINT* pStrides, UINT* pOffset)
{
	SetVertexBuffers(startSlot, { pVertexBuffer }, pStrides, pOffset);
}

void RenderSystem::SetVertexBuffers(const int startSlot, const vector<ID3D11Buffer*>& pVertexBuffers, UINT* pStrides, UINT* pOffset)
{
	m_pDeviceContext->IASetVertexBuffers(startSlot, pVertexBuffers.size(), pVertexBuffers.data(), pStrides, pOffset);
}

void RenderSystem::SetIndexBuffer(ID3D11Buffer* pIndexBuffer, const DXGI_FORMAT format /*= DXGI_FORMAT_R32_UINT*/, const UINT offset /*= 0*/)
{
	if (pIndexBuffer != m_CurrentRenderState.IndexBuffer)
	{
		m_pDeviceContext->IASetIndexBuffer(pIndexBuffer, format, offset);
		m_CurrentRenderState.IndexBuffer = pIndexBuffer;
	}
}

#pragma endregion INITIALIZATION