#include "FluxEngine.h"
#include "D3D11GraphicsImpl.h"
#include "../Graphics.h"
#include "../RasterizerState.h"
#include "../RenderTarget.h"
#include "../ShaderVariation.h"
#include "../IndexBuffer.h"
#include "../ConstantBuffer.h"
#include "../VertexBuffer.h"
#include "../Texture.h"
#include "../DepthStencilState.h"
#include "../InputLayout.h"
#include "../BlendState.h"
#include "../Shader.h"
#include "../ShaderProgram.h"
#include "UI/ImmediateUI.h"
#include "Core/Window.h"
#include "FileSystem/File/PhysicalFile.h"

Graphics::Graphics(Context* pContext, Window* pWindow) :
	Subsystem(pContext), m_pWindow(pWindow)
{
	m_pImpl = std::make_unique<GraphicsImpl>();
	for (size_t i = 0; i < m_CurrentRenderTargets.size(); ++i)
		m_CurrentRenderTargets[i] = nullptr;
}

Graphics::~Graphics()
{
	if (m_pImpl->m_pSwapChain.IsValid())
		m_pImpl->m_pSwapChain->SetFullscreenState(FALSE, NULL);

	m_pWindow->OnWindowSizeChanged().Remove(m_WindowSizeChangedHandle);

#if 0
	ComPtr<ID3D11Debug> pDebug;
	HR(m_pDevice->QueryInterface(IID_PPV_ARGS(&pDebug)));
	pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_SUMMARY);
#endif
}

bool Graphics::SetMode(
	const bool vsync,
	const int multiSample,
	const int refreshRate)
{
	AUTOPROFILE(Graphics_SetMode);

	if (m_pWindow == nullptr)
	{
		FLUX_LOG(Error, "[Graphics::Graphics] > Window is null");
		return false;
	}
	m_WindowSizeChangedHandle = m_pWindow->OnWindowSizeChanged().AddRaw(this, &Graphics::UpdateSwapchain);
	m_Width = m_pWindow->GetWidth();
	m_Height = m_pWindow->GetHeight();

	m_Vsync = vsync;
	m_RefreshRate = refreshRate;
	m_Multisample = multiSample;

	if (!m_pImpl->m_pDevice.IsValid() || m_Multisample != multiSample)
	{
		if (!CreateDevice(m_Width, m_Height))
			return false;
	}
	UpdateSwapchain(m_Width, m_Height);

	m_pBlendState = std::make_unique<BlendState>();
	m_pRasterizerState = std::make_unique<RasterizerState>();
	m_pRasterizerState->SetMultisampleEnabled(m_Multisample > 1);
	m_pDepthStencilState = std::make_unique<DepthStencilState>();

	Clear();
	m_pImpl->m_pSwapChain->Present(0, 0);

	FLUX_LOG(Info, "[Graphics::SetMode] Graphics initialized");

	return true;
}

void Graphics::SetRenderTarget(const int index, RenderTarget* pRenderTarget)
{
	if (index == 0 && pRenderTarget == nullptr)
	{
		m_CurrentRenderTargets[0] = m_pDefaultRenderTarget.get();
		m_pImpl->m_RenderTargetsDirty = true;
	}
	else if(m_CurrentRenderTargets[index] != pRenderTarget)
	{
		m_CurrentRenderTargets[index] = pRenderTarget;
		m_pImpl->m_RenderTargetsDirty = true;
	}
}

void Graphics::SetVertexBuffer(VertexBuffer* pBuffer)
{
	SetVertexBuffers({ pBuffer });
}

void Graphics::SetVertexBuffers(const std::vector<VertexBuffer*>& pBuffers, unsigned int instanceOffset)
{
	if (pBuffers.size() > GraphicsConstants::MAX_VERTEX_BUFFERS)
	{
		FLUX_LOG(Error, "[Graphics::SetVertexBuffers] > More than %i vertex buffers is not allowed", GraphicsConstants::MAX_VERTEX_BUFFERS);
		return;
	}

	for (unsigned int i = 0; i < GraphicsConstants::MAX_VERTEX_BUFFERS; ++i)
	{
		VertexBuffer* pBuffer = i >= pBuffers.size() ? nullptr : pBuffers[i];
		bool changed = false;

		if (pBuffer)
		{
			m_CurrentVertexBuffers[i] = pBuffer;
			m_pImpl->m_CurrentOffsets[i] = pBuffer->GetElements()[0].PerInstance ? instanceOffset : 0;
			m_pImpl->m_CurrentStrides[i] = pBuffer->GetVertexStride();
			m_pImpl->m_CurrentVertexBuffers[i] = (ID3D11Buffer*)pBuffer->GetBuffer();
			changed = true;
		}
		else if (m_CurrentVertexBuffers[i])
		{
			m_CurrentVertexBuffers[i] = nullptr;
			m_pImpl->m_CurrentOffsets[i] = 0;
			m_pImpl->m_CurrentStrides[i] = 0;
			m_pImpl->m_CurrentVertexBuffers[i] = nullptr;
			changed = true;
		}
		if (changed)
		{
			m_pImpl->m_VertexBuffersDirty = true;
			if (i < m_pImpl->m_FirstDirtyVertexBuffer)
				m_pImpl->m_FirstDirtyVertexBuffer = i;
			if (i > m_pImpl->m_LastDirtyVertexBuffer)
				m_pImpl->m_LastDirtyVertexBuffer = i;
		}
	}
}

void Graphics::SetIndexBuffer(IndexBuffer* pIndexBuffer)
{
	if (m_pCurrentIndexBuffer != pIndexBuffer)
	{
		if (pIndexBuffer)
			m_pImpl->m_pDeviceContext->IASetIndexBuffer((ID3D11Buffer*)pIndexBuffer->GetBuffer(), pIndexBuffer->IsSmallStride() ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
		else
			m_pImpl->m_pDeviceContext->IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
		m_pCurrentIndexBuffer = pIndexBuffer;
	}
}

bool Graphics::SetShader(const ShaderType type, ShaderVariation* pShader)
{
	if (m_CurrentShaders[(unsigned int)type] != pShader)
	{
		m_CurrentShaders[(unsigned int)type] = pShader;
		switch (type)
		{
		case ShaderType::VertexShader:
			m_pImpl->m_pDeviceContext->VSSetShader(pShader ? (ID3D11VertexShader*)pShader->GetShaderObject() : nullptr, nullptr, 0);
			break;
		case ShaderType::PixelShader:
			m_pImpl->m_pDeviceContext->PSSetShader(pShader ? (ID3D11PixelShader*)pShader->GetShaderObject() : nullptr, nullptr, 0);
			break;
		case ShaderType::GeometryShader:
			m_pImpl->m_pDeviceContext->GSSetShader(pShader ? (ID3D11GeometryShader*)pShader->GetShaderObject() : nullptr, nullptr, 0);
			break;
		case ShaderType::ComputeShader:
			m_pImpl->m_pDeviceContext->CSSetShader(pShader ? (ID3D11ComputeShader*)pShader->GetShaderObject() : nullptr, nullptr, 0);
			break;
		default:
			FLUX_LOG(Error, "[Graphics::SetShader] > Shader type not implemented");
			return false;
		}
		m_pImpl->m_ShaderProgramDirty = true;
	}

	if (pShader)
	{
		bool buffersChanged = false;
		const auto& buffers = pShader->GetConstantBuffers();
		for (unsigned int i = 0; i < buffers.size(); ++i)
		{
			if (buffers[i] != m_CurrentConstBuffers[(unsigned int)type][i])
			{
				m_CurrentConstBuffers[(unsigned int)type][i] = buffers[i] ? buffers[i]->GetBuffer() : nullptr;
				buffersChanged = true;
			}
		}
		if (buffersChanged)
		{
			switch (type)
			{
			case ShaderType::VertexShader:
				m_pImpl->m_pDeviceContext->VSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
				break;
			case ShaderType::PixelShader:
				m_pImpl->m_pDeviceContext->PSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
				break;
			case ShaderType::GeometryShader:
				m_pImpl->m_pDeviceContext->GSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
				break;
			case ShaderType::ComputeShader:
				m_pImpl->m_pDeviceContext->CSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
				break;
			default:
				break;
			}
		}
	}
	return true;
}

bool Graphics::SetShaderParameter(const std::string& name, const void* pData)
{
	if (m_pImpl->m_ShaderProgramDirty)
	{
		unsigned int hash = 0;
		for (ShaderVariation* pVariation : m_CurrentShaders)
		{
			hash <<= 8;
			if (pVariation == nullptr)
				continue;
			hash |= pVariation->GetName().size();
		}
		auto pIt = m_pImpl->m_ShaderPrograms.find(hash);
		if (pIt != m_pImpl->m_ShaderPrograms.end())
			m_pImpl->m_pCurrentShaderProgram = pIt->second.get();
		else
		{
			AUTOPROFILE(Graphics_SetShaderParameter_CreateShaderProgram);
			std::unique_ptr<ShaderProgram> pShaderProgram = std::make_unique<ShaderProgram>(m_CurrentShaders);
			m_pImpl->m_ShaderPrograms[hash] = std::move(pShaderProgram);
			m_pImpl->m_pCurrentShaderProgram = m_pImpl->m_ShaderPrograms[hash].get();
		}
		m_pImpl->m_ShaderProgramDirty = false;
	}
	return m_pImpl->m_pCurrentShaderProgram->SetParameter(name, pData);
}

void Graphics::SetViewport(const FloatRect& rect, bool relative)
{
	D3D11_VIEWPORT viewport;
	viewport.Height = rect.GetHeight();
	viewport.Width = rect.GetWidth();
	viewport.TopLeftX = rect.Left;
	viewport.TopLeftY = rect.Top;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	if (relative)
	{
		m_CurrentViewport = { viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height };

		viewport.Height *= m_Height;
		viewport.Width *= m_Width;
		viewport.TopLeftX *= m_Width;
		viewport.TopLeftY *= m_Height;
	}
	else
	{
		m_CurrentViewport = { viewport.TopLeftX / m_Width, viewport.TopLeftY / m_Height, viewport.Width / m_Width, viewport.Height / m_Height };
	}

	m_pImpl->m_pDeviceContext->RSSetViewports(1, &viewport);
}

void Graphics::SetTexture(const TextureSlot slot, Texture* pTexture)
{
	if (slot >= TextureSlot::MAX)
	{
		FLUX_LOG(Warning, "[Graphics::SetTexture] > Can't assign a texture to a slot out of range");
		return;
	}

	if (pTexture && (pTexture->GetResourceView() == m_pImpl->m_ShaderResourceViews[(unsigned int)slot] && pTexture->GetSamplerState() == m_pImpl->m_SamplerStates[(unsigned int)slot]))
		return;

	if (pTexture)
		pTexture->UpdateParameters();

	m_pImpl->m_ShaderResourceViews[(size_t)slot] = pTexture ? (ID3D11ShaderResourceView*)pTexture->GetResourceView() : nullptr;
	m_pImpl->m_SamplerStates[(size_t)slot] = pTexture ? (ID3D11SamplerState*)pTexture->GetSamplerState() : nullptr;

	m_pImpl->m_TexturesDirty = true;
	if (m_pImpl->m_FirstDirtyTexture > (unsigned int)slot)
		m_pImpl->m_FirstDirtyTexture = (unsigned int)slot;
	if (m_pImpl->m_LastDirtyTexture < (unsigned int)slot)
		m_pImpl->m_LastDirtyTexture = (unsigned int)slot;
}

void Graphics::Draw(const PrimitiveType type, const int vertexStart, const int vertexCount)
{
	PrepareDraw();

	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	unsigned int primitiveCount = 0;
	GraphicsImpl::GetPrimitiveType(type, vertexCount, topology, primitiveCount);
	if (topology != m_pImpl->m_CurrentPrimitiveType)
	{
		m_pImpl->m_CurrentPrimitiveType = topology;
		m_pImpl->m_pDeviceContext->IASetPrimitiveTopology(topology);
	}

	m_pImpl->m_pDeviceContext->Draw(vertexCount, vertexStart);

	++m_BatchCount;
	m_PrimitiveCount += primitiveCount;
}

void Graphics::DrawIndexed(const PrimitiveType type, const int indexCount, const int indexStart, const int minVertex)
{
	PrepareDraw();

	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	unsigned int primitiveCount = 0;
	GraphicsImpl::GetPrimitiveType(type, indexCount, topology, primitiveCount);
	if (topology != m_pImpl->m_CurrentPrimitiveType)
	{
		m_pImpl->m_CurrentPrimitiveType = topology;
		m_pImpl->m_pDeviceContext->IASetPrimitiveTopology(topology);
	}

	m_pImpl->m_pDeviceContext->DrawIndexed(indexCount, indexStart, minVertex);

	++m_BatchCount;
	m_PrimitiveCount += primitiveCount;
}

void Graphics::DrawIndexedInstanced(const PrimitiveType type, const int indexCount, const int indexStart, const int instanceCount, const int minVertex, const int instanceStart)
{
	PrepareDraw();

	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	unsigned int primitiveCount = 0;
	GraphicsImpl::GetPrimitiveType(type, instanceCount * indexCount, topology, primitiveCount);
	if (topology != m_pImpl->m_CurrentPrimitiveType)
	{
		m_pImpl->m_CurrentPrimitiveType = topology;
		m_pImpl->m_pDeviceContext->IASetPrimitiveTopology(topology);
	}

	m_pImpl->m_pDeviceContext->DrawIndexedInstanced(indexCount, instanceCount, indexStart, minVertex, instanceStart);

	++m_BatchCount;
	m_PrimitiveCount += primitiveCount;
}

void Graphics::Clear(const ClearFlags clearFlags, const Color& color, const float depth, const unsigned char stencil)
{
	if(m_pDefaultRenderTarget)
		m_pDefaultRenderTarget->Clear(clearFlags, color, depth, stencil);
}

void Graphics::PrepareDraw()
{
	if (m_pImpl->m_RenderTargetsDirty)
	{
		for (int i = 0; i < GraphicsConstants::MAX_RENDERTARGETS; ++i)
			m_pImpl->m_RenderTargetViews[i] = m_CurrentRenderTargets[i] ? (ID3D11RenderTargetView*)m_CurrentRenderTargets[i]->GetRenderTexture()->GetRenderTargetView() : nullptr;
		m_pImpl->m_pDepthStencilView = m_CurrentRenderTargets[0] ? (ID3D11DepthStencilView*)m_CurrentRenderTargets[0]->GetDepthTexture()->GetRenderTargetView() : nullptr;
		m_pImpl->m_pDeviceContext->OMSetRenderTargets(GraphicsConstants::MAX_RENDERTARGETS, m_pImpl->m_RenderTargetViews.data(), m_pImpl->m_pDepthStencilView);
		m_pImpl->m_RenderTargetsDirty = false;
	}

	if (m_pDepthStencilState->IsDirty())
	{
		ID3D11DepthStencilState* pState = (ID3D11DepthStencilState*)m_pDepthStencilState->GetOrCreate(this);
		m_pImpl->m_pDeviceContext->OMSetDepthStencilState(pState, m_pDepthStencilState->GetStencilRef());
	}

	if (m_pRasterizerState->IsDirty())
	{
		ID3D11RasterizerState* pState = (ID3D11RasterizerState*)m_pRasterizerState->GetOrCreate(this);
		m_pImpl->m_pDeviceContext->RSSetState(pState);
	}

	if (m_pBlendState->IsDirty())
	{
		ID3D11BlendState* pBlendState = (ID3D11BlendState*)m_pBlendState->GetOrCreate(this);
		m_pImpl->m_pDeviceContext->OMSetBlendState(pBlendState, nullptr, std::numeric_limits<unsigned int>::max());
	}

	if (m_pImpl->m_TexturesDirty)
	{
		m_pImpl->m_pDeviceContext->VSSetSamplers(m_pImpl->m_FirstDirtyTexture, m_pImpl->m_LastDirtyTexture - m_pImpl->m_FirstDirtyTexture + 1, m_pImpl->m_SamplerStates.data() + m_pImpl->m_FirstDirtyTexture);
		m_pImpl->m_pDeviceContext->PSSetSamplers(m_pImpl->m_FirstDirtyTexture, m_pImpl->m_LastDirtyTexture - m_pImpl->m_FirstDirtyTexture + 1, m_pImpl->m_SamplerStates.data() + m_pImpl->m_FirstDirtyTexture);
		m_pImpl->m_pDeviceContext->VSSetShaderResources(m_pImpl->m_FirstDirtyTexture, m_pImpl->m_LastDirtyTexture - m_pImpl->m_FirstDirtyTexture + 1, m_pImpl->m_ShaderResourceViews.data() + m_pImpl->m_FirstDirtyTexture);
		m_pImpl->m_pDeviceContext->PSSetShaderResources(m_pImpl->m_FirstDirtyTexture, m_pImpl->m_LastDirtyTexture - m_pImpl->m_FirstDirtyTexture + 1, m_pImpl->m_ShaderResourceViews.data() + m_pImpl->m_FirstDirtyTexture);

		m_pImpl->m_TexturesDirty = false;
		m_pImpl->m_FirstDirtyTexture = m_pImpl->m_FirstDirtyTexture = std::numeric_limits<unsigned int>::max();
		m_pImpl->m_LastDirtyTexture = 0;
	}

	if (m_pImpl->m_VertexBuffersDirty)
	{
		//Set the vertex buffers
		m_pImpl->m_pDeviceContext->IASetVertexBuffers(
			m_pImpl->m_FirstDirtyVertexBuffer,
			m_pImpl->m_LastDirtyVertexBuffer - m_pImpl->m_FirstDirtyVertexBuffer + 1,
			&m_pImpl->m_CurrentVertexBuffers[m_pImpl->m_FirstDirtyVertexBuffer],
			&m_pImpl->m_CurrentStrides[m_pImpl->m_FirstDirtyVertexBuffer],
			&m_pImpl->m_CurrentOffsets[m_pImpl->m_FirstDirtyVertexBuffer]);

		//Calculate the input element description hash to find the correct input layout
		unsigned long long hash = 0;
		for (VertexBuffer* pBuffer : m_CurrentVertexBuffers)
		{
			if (pBuffer)
			{
				hash <<= pBuffer->GetElements().size() * 10;
				hash |= pBuffer->GetBufferHash();
			}
			else
			{
				hash <<= 1;
			}
		}

		auto pInputLayout = m_pImpl->m_InputLayoutMap.find(hash);
		if (pInputLayout != m_pImpl->m_InputLayoutMap.end())
			m_pImpl->m_pDeviceContext->IASetInputLayout((ID3D11InputLayout*)pInputLayout->second->GetInputLayout());
		else
		{
			std::unique_ptr<InputLayout> pNewInputLayout = std::make_unique<InputLayout>(this);
			pNewInputLayout->Create(m_CurrentVertexBuffers.data(), (unsigned int)m_CurrentVertexBuffers.size(), m_CurrentShaders[(unsigned int)ShaderType::VertexShader]);
			m_pImpl->m_pDeviceContext->IASetInputLayout((ID3D11InputLayout*)pNewInputLayout->GetInputLayout());
			m_pImpl->m_InputLayoutMap[hash] = std::move(pNewInputLayout);
		}

		m_pImpl->m_FirstDirtyVertexBuffer = std::numeric_limits<unsigned int>::max();
		m_pImpl->m_LastDirtyVertexBuffer = 0;
		m_pImpl->m_VertexBuffersDirty = false;
	}

	if (m_ScissorRectDirty)
	{
		D3D11_RECT rect = {
			(LONG)m_CurrentScissorRect.Left,
			(LONG)m_CurrentScissorRect.Top,
			(LONG)m_CurrentScissorRect.Right,
			(LONG)m_CurrentScissorRect.Bottom };
		m_pImpl->m_pDeviceContext->RSSetScissorRects(1, &rect);
		m_ScissorRectDirty = false;
	}

	for (size_t i = 0; i < (size_t)ShaderType::MAX; ++i)
	{
		ShaderVariation* pShader = m_CurrentShaders[i];
		if (pShader == nullptr)
			continue;
		for (ConstantBuffer* pBuffer : pShader->GetConstantBuffers())
		{
			if (pBuffer)
				pBuffer->Apply();
		}
	}
}

void Graphics::BeginFrame()
{
	m_BatchCount = 0;
	m_PrimitiveCount = 0;
}

void Graphics::EndFrame()
{
	m_pImpl->m_pSwapChain->Present(m_Vsync ? 1 : 0, 0);
}

bool Graphics::EnumerateAdapters()
{
	AUTOPROFILE(Graphics_EnumerateAdapters);

	//Create the factor
	HR(CreateDXGIFactory(IID_PPV_ARGS(m_pImpl->m_pFactory.GetAddressOf())));
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

	//Create the device
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	HR(D3D11CreateDevice(
		m_pImpl->m_pAdapter.Get(),
		D3D_DRIVER_TYPE_UNKNOWN,
		nullptr,
		createDeviceFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		m_pImpl->m_pDevice.GetAddressOf(),
		&featureLevel,
		m_pImpl->m_pDeviceContext.GetAddressOf())
	);

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		FLUX_LOG(Error, "[Graphics::CreateDevice()] > Feature Level 11_0 not supported!");
		return false;
	}

	if (!m_pImpl->CheckMultisampleQuality(DXGI_FORMAT_B8G8R8A8_UNORM, m_Multisample))
		m_Multisample = 1;

	m_pImpl->m_pSwapChain.Reset();

	AUTOPROFILE(Graphics_CreateSwapchain);

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
	swapDesc.SampleDesc.Quality = m_pImpl->GetMultisampleQuality(DXGI_FORMAT_R8G8B8A8_UNORM, m_Multisample);
	swapDesc.OutputWindow = m_pWindow->GetHandle();
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = m_pWindow->GetType() != WindowType::FULLSCREEN;

	//Create the swap chain
	HR(m_pImpl->m_pFactory->CreateSwapChain(m_pImpl->m_pDevice.Get(), &swapDesc, m_pImpl->m_pSwapChain.GetAddressOf()));

	return true;
}

void Graphics::UpdateSwapchain(int width, int height)
{
	AUTOPROFILE(Graphics_UpdateSwapchain);

	m_Width = width;
	m_Height = height;

	if (!m_pImpl->m_pSwapChain.IsValid())
		return;

	assert(m_pImpl->m_pDevice.IsValid());
	assert(m_pImpl->m_pSwapChain.IsValid());

	m_pImpl->m_pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_pDefaultRenderTarget.reset();

	HR(m_pImpl->m_pSwapChain->ResizeBuffers(1, m_Width, m_Height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	ID3D11Texture2D *pBackbuffer = nullptr;
	HR(m_pImpl->m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackbuffer)));

	RenderTargetDesc desc = {};
	desc.Width = m_Width;
	desc.Height = m_Height;
	desc.pColorResource = pBackbuffer;
	desc.pDepthResource = nullptr;
	desc.ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DepthFormat = DXGI_FORMAT_R24G8_TYPELESS;
	desc.MultiSample = m_Multisample;
	m_pDefaultRenderTarget = std::make_unique<RenderTarget>(m_pContext);
	m_pDefaultRenderTarget->Create(desc);

	m_pImpl->m_pDepthStencilView = nullptr;
	for (size_t i = 0; i < m_CurrentRenderTargets.size(); ++i)
	{
		m_CurrentRenderTargets[i] = nullptr;
		m_pImpl->m_RenderTargetViews[i] = nullptr;
	}
	m_pImpl->m_RenderTargetsDirty = true;

	SetRenderTarget(0, m_pDefaultRenderTarget.get());
	SetViewport(m_CurrentViewport, true);
}

void Graphics::TakeScreenshot()
{
	std::stringstream str;
	str << Paths::ScreenshotFolder << "\\" << GetTimeStamp() << ".png";
	PhysicalFile file(str.str());
	if (!file.Open(FileMode::Write, ContentType::Binary))
		return;
	m_pDefaultRenderTarget->GetRenderTexture()->Save(file);
	file.Close();
}

ConstantBuffer* Graphics::GetOrCreateConstantBuffer(const std::string& name, unsigned int size)
{
	auto pIt = m_ConstantBuffers.find(name);
	if (pIt != m_ConstantBuffers.end())
	{
		if ((unsigned int)pIt->second->GetSize() != size)
		{
			FLUX_LOG(Error, "[Graphics::GetOrCreateConstantBuffer] > Constant buffer with name '%s' already exists but with a different size", name.c_str());
			return nullptr;
		}
		return pIt->second.get();
	}
	std::unique_ptr<ConstantBuffer> pBuffer = std::make_unique<ConstantBuffer>(this);
	pBuffer->SetSize(size);
	m_ConstantBuffers[name] = std::move(pBuffer);
	return m_ConstantBuffers[name].get();
}