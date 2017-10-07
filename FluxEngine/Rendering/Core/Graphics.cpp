#include "stdafx.h"
#include "Graphics.h"
#include "RenderTarget.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ShaderVariation.h"
#include "InputLayout.h"
#include "Texture.h"
#include "ConstantBuffer.h"

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
	if (!UpdateSwapchain(width, height))
		return false;


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
	if (enabled != m_ScissorEnabled)
	{
		m_ScissorEnabled = enabled;
		m_RasterizerStateDirty = true;
	}
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

void Graphics::SetFillMode(const FillMode& fillMode)
{
	if (fillMode != m_FillMode)
	{
		m_FillMode = fillMode;
		m_RasterizerStateDirty = true;
	}
}

void Graphics::SetCullMode(const CullMode& cullMode)
{
	if (cullMode != m_CullMode)
	{
		m_CullMode = cullMode;
		m_RasterizerStateDirty = true;
	}
}

void Graphics::SetBlendMode(const BlendMode& blendMode, const bool alphaToCoverage)
{
	if (blendMode != m_BlendMode || alphaToCoverage != m_AlphaToCoverage)
	{
		m_BlendMode = blendMode;
		m_AlphaToCoverage = alphaToCoverage;
		m_BlendStateDirty = true;
	}
}

void Graphics::SetColorWrite(const ColorWrite colorWriteMask /*= ColorWrite::ALL*/)
{
	if (m_ColorWriteMask !=colorWriteMask)
	{
		m_ColorWriteMask = colorWriteMask;
		m_BlendStateDirty = true;
	}
}

void Graphics::SetDepthEnabled(const bool enabled)
{
	if (enabled != m_DepthEnabled)
	{
		m_DepthEnabled = enabled;
		m_DepthStencilStateDirty = true;
	}
}

void Graphics::SetDepthTest(const CompareMode& comparison)
{
	if (comparison != m_DepthCompareMode)
	{
		m_DepthCompareMode = comparison;
		m_DepthStencilStateDirty = true;
	}
}

void Graphics::SetStencilTest(bool stencilEnabled, const CompareMode mode, const StencilOperation pass, const StencilOperation fail, const StencilOperation zFail, const unsigned int stencilRef, const unsigned char compareMask, unsigned char writeMask)
{
	if (stencilEnabled != m_StencilTestEnabled)
	{
		m_StencilTestEnabled = stencilEnabled;
		m_DepthStencilStateDirty = true;
	}
	if (mode != m_StencilTestMode)
	{
		m_StencilTestMode = mode;
		m_DepthStencilStateDirty = true;
	}
	if (pass != m_StencilTestPassOperation)
	{
		m_StencilTestPassOperation = pass;
		m_DepthStencilStateDirty = true;
	}
	if (fail != m_StencilTestFailOperation)
	{
		m_StencilTestFailOperation = fail;
		m_DepthStencilStateDirty = true;
	}
	if (zFail != m_StencilTestZFailOperation)
	{
		m_StencilTestZFailOperation = zFail;
		m_DepthStencilStateDirty = true;
	}
	if (stencilRef != m_StencilRef)
	{
		m_StencilRef = stencilRef;
		m_DepthStencilStateDirty = true;
	}
	if (compareMask != m_StencilCompareMask)
	{
		m_StencilCompareMask = compareMask;
		m_DepthStencilStateDirty = true;
	}
	if (writeMask != m_StencilWriteMask)
	{
		m_StencilWriteMask = writeMask;
		m_DepthStencilStateDirty = true;
	}
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
	if (m_RasterizerStateDirty)
	{
		UpdateRasterizerState();
		m_RasterizerStateDirty = false;
	}
	if (m_BlendStateDirty)
	{
		UpdateBlendState();
		m_BlendStateDirty = false;
	}
	if (m_DepthStencilStateDirty)
	{
		UpdateDepthStencilState();
		m_DepthStencilStateDirty = false;
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
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.SampleDesc.Count = m_Multisample;
	swapDesc.SampleDesc.Quality = GetMultisampleQuality(DXGI_FORMAT_R8G8B8A8_UNORM, m_Multisample);
	swapDesc.OutputWindow = m_Hwnd;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = m_WindowType != WindowType::FULLSCREEN;

	//Create the swap chain
	HR(m_pFactory->CreateSwapChain(m_pDevice.Get(), &swapDesc, m_pSwapChain.GetAddressOf()));

	return true;
}

bool Graphics::UpdateSwapchain(const int windowWidth, const int windowHeight)
{
	AUTOPROFILE(UpdateSwapchain);

	assert(m_pDevice.IsValid());
	assert(m_pSwapChain.IsValid());

	m_pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_pDefaultRenderTarget.reset();

	HR(m_pSwapChain->ResizeBuffers(1, windowWidth, windowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	ID3D11Texture2D *pBackbuffer = nullptr;
	HR(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackbuffer)));

	RENDER_TARGET_DESC desc = {};
	desc.Width = windowWidth;
	desc.Height = windowHeight;
	desc.pColor = pBackbuffer;
	desc.pDepth = nullptr;
	desc.ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DepthFormat = DXGI_FORMAT_R24G8_TYPELESS;
	desc.MsaaSampleCount = m_Multisample;
	m_pDefaultRenderTarget = unique_ptr<RenderTarget>(new RenderTarget(this));
	m_pDefaultRenderTarget->Create(desc);
	SetRenderTarget(m_pDefaultRenderTarget.get());

	m_WindowWidth = windowWidth;
	m_WindowHeight = windowHeight;
	SetViewport(m_CurrentViewport);

	return true;
}

void Graphics::UpdateRasterizerState()
{
	m_pRasterizerState.Reset();

	D3D11_RASTERIZER_DESC desc = {};
	desc.AntialiasedLineEnable = false;
	switch (m_CullMode)
	{
	case CullMode::FRONT:
		desc.CullMode = D3D11_CULL_FRONT;
		break;
	case CullMode::BACK:
		desc.CullMode = D3D11_CULL_BACK;
		break;
	case CullMode::NONE:
		desc.CullMode = D3D11_CULL_NONE;
		break;
	}

	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0.0f;
	desc.DepthClipEnable = true;
	switch (m_FillMode)
	{
	case FillMode::SOLID:
		desc.FillMode = D3D11_FILL_SOLID;
		break;
	case FillMode::WIREFRAME:
		desc.FillMode = D3D11_FILL_WIREFRAME;
		break;
	}
	desc.FrontCounterClockwise = false;
	desc.MultisampleEnable = m_Multisample > 1;
	desc.ScissorEnable = m_ScissorEnabled;
	desc.SlopeScaledDepthBias = 0.0f;

	HR(m_pDevice->CreateRasterizerState(&desc, m_pRasterizerState.GetAddressOf()));

	m_pDeviceContext->RSSetState(m_pRasterizerState.Get());
}

void Graphics::UpdateBlendState()
{
	m_pBlendState.Reset();

	D3D11_BLEND_DESC desc = {};
	desc.AlphaToCoverageEnable = m_AlphaToCoverage;
	desc.IndependentBlendEnable = false;
	desc.RenderTarget[0].BlendEnable = m_BlendMode == BlendMode::REPLACE ? false : true;
	desc.RenderTarget[0].RenderTargetWriteMask = (unsigned int)m_ColorWriteMask;
	
	switch (m_BlendMode)
	{
	case BlendMode::REPLACE:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::ADD:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::MULTIPLY:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_COLOR;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::ALPHA:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::ADDALPHA:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::PREMULALPHA:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::INVDESTALPHA:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_DEST_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::SUBTRACT:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_REV_SUBTRACT;
		break;
	case BlendMode::SUBTRACTALPHA:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_REV_SUBTRACT;
		break;
	}

	HR(m_pDevice->CreateBlendState(&desc, m_pBlendState.GetAddressOf()));

	m_pDeviceContext->OMSetBlendState(m_pBlendState.Get(), nullptr, numeric_limits<unsigned int>::max());
}

void Graphics::UpdateDepthStencilState()
{
	m_pDepthStencilState.Reset();

	D3D11_DEPTH_STENCIL_DESC desc = {};
	desc.DepthEnable = m_DepthEnabled;
	desc.DepthWriteMask = m_DepthEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	switch (m_DepthCompareMode)
	{
	case CompareMode::ALWAYS:
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		break;
	case CompareMode::EQUAL:
		desc.DepthFunc = D3D11_COMPARISON_EQUAL;
		break;
	case CompareMode::NOTEQUAL:
		desc.DepthFunc = D3D11_COMPARISON_NOT_EQUAL;
		break;
	case CompareMode::LESS:
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		break;
	case CompareMode::LESSEQUAL:
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		break;
	case CompareMode::GREATER:
		desc.DepthFunc = D3D11_COMPARISON_GREATER;
		break;
	case CompareMode::GREATEREQUAL:
		desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
		break;
	}

	desc.StencilEnable = m_StencilTestEnabled;
	desc.StencilReadMask = m_StencilCompareMask;
	desc.StencilWriteMask = m_StencilWriteMask;

	switch (m_StencilTestMode)
	{
	case CompareMode::ALWAYS:
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		break;
	case CompareMode::EQUAL:
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
		break;
	case CompareMode::NOTEQUAL:
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
		break;
	case CompareMode::LESS:
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_LESS;
		break;
	case CompareMode::LESSEQUAL:
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL;
		break;
	case CompareMode::GREATER:
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_GREATER;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_GREATER;
		break;
	case CompareMode::GREATEREQUAL:
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_GREATER_EQUAL;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_GREATER_EQUAL;
		break;
	}
	switch (m_StencilTestPassOperation)
	{
	case StencilOperation::KEEP:
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		break;
	case StencilOperation::ZERO:
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
		break;
	case StencilOperation::REF:
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		break;
	case StencilOperation::INCR:
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
		break;
	case StencilOperation::DECR:
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_DECR;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_DECR;
		break;
	}

	switch (m_StencilTestFailOperation)
	{
	case StencilOperation::KEEP:
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		break;
	case StencilOperation::ZERO:
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
		break;
	case StencilOperation::REF:
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;
		break;
	case StencilOperation::INCR:
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_INCR;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_INCR;
		break;
	case StencilOperation::DECR:
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_DECR;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_DECR;
		break;
	}

	switch (m_StencilTestZFailOperation)
	{
	case StencilOperation::KEEP:
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		break;
	case StencilOperation::ZERO:
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
		break;
	case StencilOperation::REF:
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
		break;
	case StencilOperation::INCR:
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		break;
	case StencilOperation::DECR:
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		break;
	}

	HR(m_pDevice->CreateDepthStencilState(&desc, m_pDepthStencilState.GetAddressOf()));

	m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilState.Get(), m_StencilRef);
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