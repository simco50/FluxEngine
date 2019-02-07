#include "FluxEngine.h"
#include "D3D12GraphicsImpl.h"
#include "../Graphics.h"
#include "../RasterizerState.h"
#include "../RenderTarget.h"
#include "../ShaderVariation.h"
#include "../IndexBuffer.h"
#include "../ConstantBuffer.h"
#include "../VertexBuffer.h"
#include "../Texture2D.h"
#include "../DepthStencilState.h"
#include "../BlendState.h"
#include "../ShaderProgram.h"
#include "../../../FileSystem/File/PhysicalFile.h"
#include "../../../Content/Image.h"
#include "../../Renderer.h"
#include "../../Geometry.h"
#include "../StructuredBuffer.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include "d3dx12.h"

std::string Graphics::m_ShaderExtension = ".hlsl";
const int Graphics::RENDERTARGET_FORMAT = (int)DXGI_FORMAT_R8G8B8A8_UNORM;
const int Graphics::DEPTHSTENCIL_FORMAT = (int)DXGI_FORMAT_D24_UNORM_S8_UINT;

Graphics::Graphics(Context* pContext)
	: Subsystem(pContext),
	m_WindowType(WindowType::WINDOWED),
	m_pImpl(std::make_unique<GraphicsImpl>())
{
	AUTOPROFILE(Graphics_Construct);

	for (RenderTarget*& pRt : m_CurrentRenderTargets)
	{
		pRt = nullptr;
	}

	pContext->InitSDLSystem(SDL_INIT_VIDEO);
}

Graphics::~Graphics()
{
	if (m_pImpl->m_pSwapChain.IsValid())
	{
		m_pImpl->m_pSwapChain->SetFullscreenState(FALSE, nullptr);
	}

	m_pContext->ShutdownSDL();

#if 0
	ComPtr<ID3D11Debug> pDebug;
	HR(pDevice->QueryInterface(IID_PPV_ARGS(pDebug.GetAddressOf())));
	pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif
}

HWND Graphics::GetWindow() const
{
	SDL_SysWMinfo sysInfo;
	SDL_VERSION(&sysInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysInfo);
	return sysInfo.info.win.window;
}

bool Graphics::SetMode(const GraphicsCreateInfo& createInfo)
{
	AUTOPROFILE(Graphics_SetMode);

	m_WindowWidth = createInfo.WindowWidth;
	m_WindowHeight = createInfo.WindowHeight;
	m_WindowType = createInfo.WindowType;
	m_Resizable = createInfo.Resizable;
	m_WindowTitle = createInfo.Title;
	m_Vsync = createInfo.VSync;
	m_RefreshRate = createInfo.RefreshRate;
	m_Multisample = createInfo.MultiSample;

	if (!OpenWindow())
	{
		return false;
	}

	if (!m_pImpl->m_pDevice.IsValid())
	{
		if (!CreateDevice(m_WindowWidth, m_WindowHeight))
		{
			return false;
		}
	}
	UpdateSwapchain(m_WindowWidth, m_WindowHeight);

	m_RasterizerState.SetMultisampleEnabled(m_Multisample > 1);

	FLUX_LOG(Info, "[Graphics::SetMode] Graphics initialized");

	return true;
}

bool Graphics::OpenWindow()
{
	AUTOPROFILE(Graphics_OpenWindow);

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);

	unsigned flags = 0;
	if (m_Resizable)
	{
		flags |= SDL_WINDOW_RESIZABLE;
	}
	switch (m_WindowType)
	{
	case WindowType::BORDERLESS:
		flags |= SDL_WINDOW_BORDERLESS;
		m_WindowWidth = displayMode.w;
		m_WindowHeight = displayMode.h;
		break;
	case WindowType::FULLSCREEN:
		flags |= SDL_WINDOW_FULLSCREEN;
		break;
	case WindowType::WINDOWED:
	default:
		break;
	}
	m_pWindow = SDL_CreateWindow(m_WindowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_WindowWidth, m_WindowHeight, flags);
	if (!m_pWindow)
	{
		return false;
	}

	//Hardcode the logo into the binary
	unsigned int pLogo[] =
	{
		0x8ba28d3a, 0xfba28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa08c3b, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xfba28d3a, 0x8ba28d3a,
		0xfba28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa89548, 0xffb6a666, 0xffd6d0b8, 0xff917f35, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xfba28d3a,
		0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffb09f5c, 0xffb1a988, 0xff897731, 0xff7c6c2c, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a,
		0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa79346, 0xffc7be9a, 0xffbcb28d, 0xffb3ab8a, 0xffb2aa89, 0xffbcb28d, 0xffc6bc93, 0xffa08b39, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a,
		0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffb7a768, 0xff958b62, 0xff77682b, 0xffc6c1aa, 0xffc6c0aa, 0xff7d6d2d, 0xffb9b190, 0xff7d6d2c, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a,
		0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa38e3d, 0xffd2caaa, 0xffa0925c, 0xfffaf9f5, 0xfff8f7f4, 0xffb4ac8a, 0xffa19772, 0xff706227, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a,
		0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffbdae74, 0xff8b7f52, 0xffb1a576, 0xff877b4b, 0xffaea787, 0xff6e6027, 0xff8f7c33, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a,
		0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffc9bf94, 0xff867a4b, 0xfffefefe, 0xffa69a6b, 0xffb3a46a, 0xff998f67, 0xffa18c39, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a,
		0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffaa984d, 0xffb0a887, 0xff716228, 0xffa09156, 0xff6f7e7d, 0xff7997b0, 0xffcdc5a9, 0xff968339, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a,
		0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffcbc19a, 0xff757d70, 0xff7c8574, 0xff879998, 0xff6ea6f3, 0xff6ea6f4, 0xff909482, 0xffb6ad8a, 0xffa08b39, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a,
		0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffb09e58, 0xffa7a48c, 0xff6ea2e8, 0xff6ea6f4, 0xff6ea7f5, 0xff6ea7f5, 0xff6e9edb, 0xff6e97c6, 0xffcbc5ac, 0xff968643, 0xffa28d3a, 0xffa28d3a, 0xffa28d3a,
		0xffa28d3a, 0xffa28d3a, 0xffa28d3a, 0xffc9c09d, 0xff718ba0, 0xff6e8dab, 0xff6e7c78, 0xff6ea7f5, 0xff6ea6f4, 0xff6ea7f5, 0xff6ea7f5, 0xff858f87, 0xffc8c2a9, 0xff9d8938, 0xffa28d3a, 0xffa28d3a,
		0xffa28d3a, 0xffa28d3a, 0xffb4a462, 0xff9e9e8c, 0xff6ea5ef, 0xff6ea5f0, 0xff6ea2e9, 0xff6ea7f5, 0xff6e99cc, 0xff6ea7f5, 0xff6ea7f5, 0xff6e9dd8, 0xffc6c0a8, 0xff9f9159, 0xffa28d3a, 0xffa28d3a,
		0xffa28d3a, 0xffa28d3a, 0xffcec49b, 0xff737762, 0xff6e848f, 0xff6e848f, 0xff6e848f, 0xff6e848f, 0xff6e848f, 0xff6e848f, 0xff6e848f, 0xff6e848f, 0xff919076, 0xffb5ae91, 0xff978336, 0xffa28d3a,
		0xfba28d3a, 0xffa28d3a, 0xffaa974c, 0xffb9b18f, 0xffb6af92, 0xffb6af92, 0xffb6af92, 0xffb6af92, 0xffb6af92, 0xffb6af92, 0xffb6af92, 0xffb6af92, 0xffaca484, 0xff746730, 0xff887630, 0xfba28d3a,
		0x8ba28d3a, 0xfba28d3a, 0xffa28d3a, 0xff9c8838, 0xff897730, 0xff877630, 0xff877630, 0xff877630, 0xff877630, 0xff877630, 0xff877630, 0xff877630, 0xff877630, 0xff8b7931, 0xfb9f8a39, 0x8ba28d3a,
	};
	SDL_Surface* pSurface = SDL_CreateRGBSurfaceFrom(pLogo, 16, 16, 4 * 8, 16 * 4, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	if (pSurface == nullptr)
	{
		return false;
	}
	SDL_SetWindowIcon(m_pWindow, pSurface);
	return true;
}

void Graphics::SetRenderTarget(const int index, RenderTarget* pRenderTarget)
{

}

void Graphics::SetDepthStencil(RenderTarget* pRenderTarget)
{

}

void Graphics::SetDepthOnly(bool enable)
{

}

void Graphics::SetVertexBuffer(VertexBuffer* pBuffer)
{
	SetVertexBuffers(&pBuffer, 1);
}

void Graphics::SetVertexBuffers(VertexBuffer** pBuffers, int bufferCount, unsigned int instanceOffset /*= 0*/)
{
	
}

void Graphics::SetIndexBuffer(IndexBuffer* pIndexBuffer)
{
	
}

void Graphics::UpdateShaders()
{
	
}

bool Graphics::SetShader(const ShaderType type, ShaderVariation* pShader)
{
	return false;
}

void Graphics::UpdateShaderProgram()
{
	
}

bool Graphics::SetShaderParameter(StringHash hash, const void* pData)
{
	UpdateShaderProgram();
	const ShaderParameter* pParameter = m_pImpl->m_pCurrentShaderProgram->GetShaderParameter(hash);
	if (pParameter == nullptr)
	{
		return false;
	}
	return pParameter->pBuffer->SetParameter(pParameter->Offset, pParameter->Size, pData);
}

bool Graphics::SetShaderParameter(StringHash hash, const void* pData, int stride, int count)
{
	UpdateShaderProgram();
	const ShaderParameter* pParameter = m_pImpl->m_pCurrentShaderProgram->GetShaderParameter(hash);
	if (pParameter == nullptr)
	{
		return false;
	}
	checkf(stride * count <= pParameter->Size, "[Graphics::SetShaderParameter] Parameter input too large");
	return pParameter->pBuffer->SetParameter(pParameter->Offset, stride * count, pData);
}

void Graphics::SetViewport(const FloatRect& rect)
{

}

void Graphics::SetTexture(const TextureSlot slot, Texture* pTexture)
{
	
}

void Graphics::SetStructuredBuffer(const TextureSlot slot, const StructuredBuffer* pBuffer)
{
	
}


void Graphics::Draw(const PrimitiveType type, const int vertexStart, const int vertexCount)
{
	
}

void Graphics::DrawIndexed(const PrimitiveType type, const int indexCount, const int indexStart, const int minVertex)
{
	
}

void Graphics::DrawIndexedInstanced(const PrimitiveType type, const int indexCount, const int indexStart, const int instanceCount, const int minVertex, const int instanceStart)
{
	
}

void Graphics::Clear(const ClearFlags clearFlags, const Color& color, const float depth, const unsigned char stencil)
{
	
}

void Graphics::FlushRenderTargetChanges(bool force)
{

}

void Graphics::FlushSRVChanges(bool force)
{
}


void Graphics::PrepareDraw()
{
}

void Graphics::BeginFrame()
{
	PROFILER_EVENT(Graphics_BeginFrame);
	m_BatchCount = 0;
	m_PrimitiveCount = 0;
}

void Graphics::EndFrame()
{
	AUTOPROFILE(Graphics_Present);
	PROFILER_EVENT(Graphics_EndFrame);
}

bool Graphics::EnumerateAdapters()
{
	AUTOPROFILE(Graphics_EnumerateAdapters);

	//Create the factor
	HR(CreateDXGIFactory1(IID_PPV_ARGS(m_pImpl->m_pFactory.GetAddressOf())));
	std::vector<IDXGIAdapter*> pAdapters;
	UINT adapterCount = 0;

	int bestAdapterIdx = 0;
	unsigned long bestMemory = 0;

	IDXGIAdapter* pAdapter = nullptr;
	FLUX_LOG(Info, "Adapters:");
	while (m_pImpl->m_pFactory->EnumAdapters(adapterCount, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		pAdapter->GetDesc(&desc);

		if (desc.DedicatedVideoMemory > bestMemory)
		{
			bestMemory = (unsigned long)desc.DedicatedVideoMemory;
			bestAdapterIdx = adapterCount;
		}

		std::wstring gpuDesc(desc.Description);
		FLUX_LOG(Info, "\t[%i] %s", adapterCount, std::string(gpuDesc.begin(), gpuDesc.end()).c_str());

		pAdapters.push_back(pAdapter);
		++adapterCount;
	}
	m_pImpl->m_pAdapter = pAdapters[bestAdapterIdx];

	return true;
}

bool Graphics::CreateDevice(const int windowWidth, const int windowHeight)
{
	AUTOPROFILE(Graphics_CreateDevice);

	EnumerateAdapters();

	{
		//Create the device
#ifdef _DEBUG
		//Enable debug
		ComPtr<ID3D12Debug> pDebugController;
		HR(D3D12GetDebugInterface(IID_PPV_ARGS(pDebugController.GetAddressOf())));
		pDebugController->EnableDebugLayer();
#endif
		D3D_FEATURE_LEVEL minimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;
		HR(D3D12CreateDevice(m_pImpl->m_pAdapter.Get(), minimumFeatureLevel, IID_PPV_ARGS(m_pImpl->m_pDevice.GetAddressOf())));
	
		m_pImpl->m_RtvDescriptorSize = (int)m_pImpl->m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_pImpl->m_DsvDescriptorSize = (int)m_pImpl->m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_pImpl->m_CbvSrvDescriptorSize = (int)m_pImpl->m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	{
		AUTOPROFILE(Graphics_CreateCommandQueue);

		//Create command queue
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		HR(m_pImpl->m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_pImpl->m_pCommandQueue.GetAddressOf())));

		if (!m_pImpl->CheckMultisampleQuality(DXGI_FORMAT_B8G8R8A8_UNORM, m_Multisample))
		{
			m_Multisample = 1;
		}
	}

	{
		AUTOPROFILE(Graphics_CreateRenderTargetHeaps);

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		rtvHeapDesc.NumDescriptors = GraphicsImpl::FRAME_COUNT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		m_pImpl->m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_pImpl->m_pRtvHeap.GetAddressOf()));

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		m_pImpl->m_pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_pImpl->m_pDsvHeap.GetAddressOf()));
	}


	m_pImpl->m_pSwapChain.Reset();

	{
		AUTOPROFILE(Graphics_CreateSwapchain);

		//Create swap chain desctriptor
		DXGI_SWAP_CHAIN_DESC swapDesc;
		swapDesc.BufferCount = GraphicsImpl::FRAME_COUNT;
		swapDesc.BufferDesc.Format = (DXGI_FORMAT)RENDERTARGET_FORMAT;
		swapDesc.BufferDesc.Height = windowHeight;
		swapDesc.BufferDesc.Width = windowWidth;
		swapDesc.BufferDesc.RefreshRate.Denominator = m_RefreshRate;
		swapDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapDesc.SampleDesc.Count = m_Multisample;
		swapDesc.SampleDesc.Quality = m_pImpl->GetMultisampleQuality((DXGI_FORMAT)RENDERTARGET_FORMAT, m_Multisample);
		swapDesc.OutputWindow = GetWindow();
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapDesc.Windowed = m_WindowType != WindowType::FULLSCREEN;

		//Create the swap chain
		HR(m_pImpl->m_pFactory->CreateSwapChain(m_pImpl->m_pCommandQueue.Get(), &swapDesc, m_pImpl->m_pSwapChain.GetAddressOf()));
	}

	return true;
}

void Graphics::UpdateSwapchain(int width, int height)
{
	AUTOPROFILE(Graphics_UpdateSwapchain);
	
	if (!m_pImpl->m_pSwapChain.IsValid())
	{
		return;
	}
	
	assert(m_pImpl->m_pDevice.IsValid());
	assert(m_pImpl->m_pSwapChain.IsValid());

	for (int i = 0; i < GraphicsImpl::FRAME_COUNT; ++i)
	{
		m_pImpl->m_RenderTargets[i].Reset();
	}
	m_pImpl->m_pDepthStencilBuffer.Reset();
	
	HR(m_pImpl->m_pSwapChain->ResizeBuffers(
		GraphicsImpl::FRAME_COUNT, 
		width, 
		height, 
		(DXGI_FORMAT)RENDERTARGET_FORMAT,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	m_pImpl->m_CurrentBackBufferIndex = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_pImpl->m_pRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < GraphicsImpl::FRAME_COUNT; ++i)
	{
		m_pImpl->m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pImpl->m_RenderTargets[i].GetAddressOf()));
		m_pImpl->m_pDevice->CreateRenderTargetView(m_pImpl->m_RenderTargets[i].Get(), nullptr, handle);
		handle.Offset(1, m_pImpl->m_RtvDescriptorSize);
	}

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = m_WindowWidth;
	desc.Height = m_WindowHeight;
	desc.DepthOrArraySize = 1;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	desc.Format = (DXGI_FORMAT)DEPTHSTENCIL_FORMAT;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = m_Multisample;
	desc.SampleDesc.Quality = m_pImpl->GetMultisampleQuality((DXGI_FORMAT)DEPTHSTENCIL_FORMAT, m_Multisample);

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = (DXGI_FORMAT)DEPTHSTENCIL_FORMAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	HR(m_pImpl->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		&clearValue,
		IID_PPV_ARGS(m_pImpl->m_pDepthStencilBuffer.GetAddressOf())));
	m_pImpl->m_pDevice->CreateDepthStencilView(m_pImpl->m_pDepthStencilBuffer.Get(), nullptr, m_pImpl->m_pDsvHeap->GetCPUDescriptorHandleForHeapStart());
	
	m_CurrentViewport.Left = 0;
	m_CurrentViewport.Top = 0;
	m_CurrentViewport.Right = (float)width;
	m_CurrentViewport.Bottom = (float)height;
	m_CurrentScissorRect = m_CurrentViewport;

	m_WindowWidth = width;
	m_WindowHeight = height;
}

void Graphics::OnResize(const int width, const int height)
{
	if (width != m_WindowWidth || height != m_WindowHeight)
	{
		UpdateSwapchain(width, height);
	}
}

void Graphics::TakeScreenshot()
{
	std::stringstream str;
	str << Paths::ScreenshotDir() << "\\Screenshot_" << DateTime::Now().ToString("%y-%m-%d_%H-%M-%S") << ".png";
	PhysicalFile file(str.str());
	if (!file.OpenWrite())
	{
		return;
	}
	TakeScreenshot(file);
}

void Graphics::TakeScreenshot(OutputStream& outputStream)
{
}

ConstantBuffer* Graphics::GetOrCreateConstantBuffer(unsigned int index, unsigned int size)
{
	return nullptr;
}

bool Graphics::GetAdapterInfo(AdapterInfo& adapterInfo)
{
	if (m_pImpl && m_pImpl->m_pAdapter)
	{
		DXGI_ADAPTER_DESC desc;
		m_pImpl->m_pAdapter->GetDesc(&desc);
		std::wstring name = desc.Description;
		adapterInfo.Name = std::string(name.begin(), name.end());
		adapterInfo.VideoMemory = desc.DedicatedVideoMemory;
		adapterInfo.SystemMemory = desc.DedicatedSystemMemory;
		adapterInfo.DeviceId = desc.DeviceId;
		adapterInfo.VendorId = desc.VendorId;
		return true;
	}
	return false;
}