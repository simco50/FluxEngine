#include "FluxEngine.h"
#include "D3D11GraphicsImpl.h"
#include "../Graphics.h"
#include "../RasterizerState.h"
#include "../RenderTarget.h"
#include "../ShaderVariation.h"
#include "../IndexBuffer.h"
#include "../ConstantBuffer.h"
#include "../VertexBuffer.h"
#include "../Texture2D.h"
#include "../DepthStencilState.h"
#include "../D3D11/D3D11InputLayout.h"
#include "../BlendState.h"
#include "../ShaderProgram.h"
#include "../../../FileSystem/File/PhysicalFile.h"
#include "../../../Content/Image.h"
#include "../../Renderer.h"
#include "../../Geometry.h"
#include "../StructuredBuffer.h"

#include <SDL.h>
#include <SDL_syswm.h>

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

	Clear();
	m_pImpl->m_pSwapChain->Present(0, 0);

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
	if (index == 0 && pRenderTarget == nullptr)
	{
		m_CurrentRenderTargets[0] = nullptr;
		m_pImpl->m_RenderTargetsDirty = true;
	}
	else if(m_CurrentRenderTargets[index] != pRenderTarget)
	{
		m_CurrentRenderTargets[index] = pRenderTarget;

		if (pRenderTarget && pRenderTarget->GetParentTexture()->GetMultiSample() > 1)
		{
			pRenderTarget->GetParentTexture()->SetResolveDirty(true);
		}

		m_pImpl->m_RenderTargetsDirty = true;
	}
}

void Graphics::SetDepthStencil(RenderTarget* pRenderTarget)
{
	if (pRenderTarget != m_pCurrentDepthStencil)
	{
		m_pCurrentDepthStencil = pRenderTarget;
		m_pImpl->m_RenderTargetsDirty = true;
	}
}

void Graphics::SetDepthOnly(bool enable)
{
	if (enable != m_RenderDepthOnly)
	{
		m_RenderDepthOnly = enable;
		m_pImpl->m_RenderTargetsDirty = true;
	}
}

void Graphics::SetVertexBuffer(VertexBuffer* pBuffer)
{
	SetVertexBuffers(&pBuffer, 1);
}

void Graphics::SetVertexBuffers(VertexBuffer** pBuffers, int bufferCount, unsigned int instanceOffset /*= 0*/)
{
	checkf(bufferCount <= GraphicsConstants::MAX_VERTEX_BUFFERS, "Vertex buffer count exceeded");

	for (int i = 0; i < GraphicsConstants::MAX_VERTEX_BUFFERS; ++i)
	{
		VertexBuffer* pBuffer = i >= bufferCount ? nullptr : pBuffers[i];
		bool changed = false;

		if (pBuffer)
		{
			if (m_CurrentVertexBuffers[i] != pBuffer)
			{
				m_CurrentVertexBuffers[i] = pBuffer;
				m_pImpl->m_CurrentOffsets[i] = pBuffer->GetElements()[0].PerInstance ? instanceOffset : 0;
				m_pImpl->m_CurrentStrides[i] = pBuffer->GetElementStride();
				m_pImpl->m_CurrentVertexBuffers[i] = (ID3D11Buffer*)pBuffer->GetResource();
				changed = true;
			}
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
			m_pImpl->m_FirstDirtyVertexBuffer = Math::Min((uint32)i, m_pImpl->m_FirstDirtyVertexBuffer);
			m_pImpl->m_LastDirtyVertexBuffer = Math::Max((uint32)i, m_pImpl->m_LastDirtyVertexBuffer);
		}
	}
}

void Graphics::SetIndexBuffer(IndexBuffer* pIndexBuffer)
{
	if (m_pCurrentIndexBuffer != pIndexBuffer)
	{
		AUTOPROFILE(Graphics_SetIndexBuffer);
		if (pIndexBuffer)
		{
			m_pImpl->m_pDeviceContext->IASetIndexBuffer((ID3D11Buffer*)pIndexBuffer->GetResource(), pIndexBuffer->GetElementStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
		}
		else
		{
			m_pImpl->m_pDeviceContext->IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
		}
		m_pCurrentIndexBuffer = pIndexBuffer;
	}
}

void Graphics::UpdateShaders()
{
	AUTOPROFILE(Graphics_UpdateShaders);

	if (m_DirtyShaders.AnyBitSet())
	{
		for(BitField<(size_t)ShaderType::MAX>::SetBitsIterator iterator = m_DirtyShaders.GetSetBitsIterator(); iterator.Valid(); ++iterator)
		{
			ShaderType type = (ShaderType)iterator.Value();
			ShaderVariation* pShader = m_CurrentShaders[(unsigned int)type];
			AUTOPROFILE_DESC(Graphics_UpdateShader, pShader ? pShader->GetName() : "None");

			switch (type)
			{
			case ShaderType::VertexShader:
				m_pImpl->m_pDeviceContext->VSSetShader(pShader ? (ID3D11VertexShader*)pShader->GetResource() : nullptr, nullptr, 0);
				break;
			case ShaderType::PixelShader:
				m_pImpl->m_pDeviceContext->PSSetShader(pShader ? (ID3D11PixelShader*)pShader->GetResource() : nullptr, nullptr, 0);
				break;

#ifdef SHADER_GEOMETRY_ENABLE
			case ShaderType::GeometryShader:
				m_pImpl->m_pDeviceContext->GSSetShader(pShader ? (ID3D11GeometryShader*)pShader->GetResource() : nullptr, nullptr, 0);
				break;
#endif
#ifdef SHADER_COMPUTE_ENABLE
			case ShaderType::ComputeShader:
				m_pImpl->m_pDeviceContext->CSSetShader(pShader ? (ID3D11ComputeShader*)pShader->GetResource() : nullptr, nullptr, 0);
				break;
#endif
#ifdef SHADER_TESSELLATION_ENABLE
			case ShaderType::DomainShader:
				m_pImpl->m_pDeviceContext->DSSetShader(pShader ? (ID3D11DomainShader*)pShader->GetResource() : nullptr, nullptr, 0);
				break;
			case ShaderType::HullShader:
				m_pImpl->m_pDeviceContext->HSSetShader(pShader ? (ID3D11HullShader*)pShader->GetResource() : nullptr, nullptr, 0);
				break;
#endif
			default:
				FLUX_LOG(Error, "[Graphics::SetShader] > Shader type not implemented");
				return;
			}

			if (pShader)
			{
				AUTOPROFILE_DESC(Graphics_SetConstantBuffers, pShader->GetName());
				bool buffersChanged = false;
				const auto& buffers = pShader->GetConstantBuffers();
				for (unsigned int i = 0; i < buffers.size(); ++i)
				{
					if (buffers[i] != m_CurrentConstBuffers[(unsigned int)type][i])
					{
						m_CurrentConstBuffers[(unsigned int)type][i] = buffers[i] ? buffers[i]->GetResource() : nullptr;
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
#ifdef SHADER_GEOMETRY_ENABLE
					case ShaderType::GeometryShader:
						m_pImpl->m_pDeviceContext->GSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
						break;
#endif
#ifdef SHADER_COMPUTE_ENABLE
					case ShaderType::ComputeShader:
						m_pImpl->m_pDeviceContext->CSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
						break;
#endif
#ifdef SHADER_TESSELLATION_ENABLE
					case ShaderType::DomainShader:
						m_pImpl->m_pDeviceContext->DSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
						break;
					case ShaderType::HullShader:
						m_pImpl->m_pDeviceContext->HSSetConstantBuffers(0, (unsigned int)ShaderParameterType::MAX, (ID3D11Buffer**)&m_CurrentConstBuffers[(unsigned int)type]);
						break;
#endif
					default:
						break;
					}
				}
			}

		};
		m_DirtyShaders.ClearAll();
	}
}

bool Graphics::SetShader(const ShaderType type, ShaderVariation* pShader)
{
	if (m_CurrentShaders[(unsigned int)type] != pShader)
	{
		m_CurrentShaders[(unsigned int)type] = pShader;
		m_DirtyShaders.SetBit((unsigned int)type);
		m_pImpl->m_ShaderProgramDirty = true;
	}
	return true;
}

void Graphics::UpdateShaderProgram()
{
	if (m_pImpl->m_ShaderProgramDirty)
	{
		AUTOPROFILE(Graphics_UpdateShaderProgram);

		uint64 hash = 0;
		int shiftAmount = (int)floor(64 / (int)ShaderType::MAX);
		int maxSize = (int)pow(2, shiftAmount);
		for (ShaderVariation* pVariation : m_CurrentShaders)
		{
			hash <<= shiftAmount;
			if (pVariation == nullptr)
			{
				continue;
			}
			hash |= (int)pVariation->GetByteCode().size() % maxSize;
		}
		auto pIt = m_pImpl->m_ShaderPrograms.find(hash);
		if (pIt != m_pImpl->m_ShaderPrograms.end())
		{
			m_pImpl->m_pCurrentShaderProgram = pIt->second.get();
		}
		else
		{
			AUTOPROFILE(Graphics_SetShaderParameter_CreateShaderProgram);
			std::unique_ptr<ShaderProgram> pShaderProgram = std::make_unique<ShaderProgram>(m_CurrentShaders);
			m_pImpl->m_ShaderPrograms[hash] = std::move(pShaderProgram);
			m_pImpl->m_pCurrentShaderProgram = m_pImpl->m_ShaderPrograms[hash].get();
		}
		m_pImpl->m_ShaderProgramDirty = false;
	}
}

bool Graphics::SetShaderParameter(StringHash hash, const void* pData)
{
	UpdateShaderProgram();
	const ShaderParameter* pParameter = m_pImpl->m_pCurrentShaderProgram->GetShaderParameter(hash);
	if (pParameter == nullptr)
	{
		return false;
	}
	return pParameter->pBuffer->SetData(pData, pParameter->Offset, pParameter->Size, false);
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
	return pParameter->pBuffer->SetData(pData, pParameter->Offset, stride * count, false);
}

void Graphics::SetViewport(const FloatRect& rect)
{
	D3D11_VIEWPORT viewport;
	viewport.Height = rect.GetHeight();
	viewport.Width = rect.GetWidth();
	viewport.TopLeftX = rect.Left;
	viewport.TopLeftY = rect.Top;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_CurrentViewport = { viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height };

	m_pImpl->m_pDeviceContext->RSSetViewports(1, &viewport);
}

void Graphics::SetTexture(const TextureSlot slot, Texture* pTexture)
{
	checkf(slot < TextureSlot::MAX, "[Graphics::SetTexture] > Can't assign a texture to a slot out of range");

	if (pTexture && (pTexture->GetResourceView() == m_pImpl->m_ShaderResourceViews[(unsigned int)slot] && pTexture->GetSamplerState() == m_pImpl->m_SamplerStates[(unsigned int)slot]))
	{
		return;
	}

	if (pTexture)
	{
		pTexture->UpdateParameters();
		pTexture->Resolve(false);
	}

	m_pImpl->m_ShaderResourceViews[(size_t)slot] = pTexture ? (ID3D11ShaderResourceView*)pTexture->GetResourceView() : nullptr;
	m_pImpl->m_SamplerStates[(size_t)slot] = pTexture ? (ID3D11SamplerState*)pTexture->GetSamplerState() : nullptr;

	m_pImpl->m_TexturesDirty = true;
	m_pImpl->m_FirstDirtyTexture = Math::Min(m_pImpl->m_FirstDirtyTexture, (int)slot);
	m_pImpl->m_LastDirtyTexture = Math::Max(m_pImpl->m_LastDirtyTexture, (int)slot);
}

void Graphics::SetStructuredBuffer(const TextureSlot slot, const StructuredBuffer* pBuffer)
{
	if (slot >= TextureSlot::MAX)
	{
		FLUX_LOG(Warning, "[Graphics::SetStructuredBuffer] > Can't assign a structuredbuffer to a slot out of range");
		return;
	}

	if (pBuffer && (pBuffer->GetView() == m_pImpl->m_ShaderResourceViews[(unsigned int)slot]))
	{
		return;
	}

	m_pImpl->m_ShaderResourceViews[(size_t)slot] = pBuffer ? (ID3D11ShaderResourceView*)pBuffer->GetView() : nullptr;
	m_pImpl->m_SamplerStates[(size_t)slot] = nullptr;

	m_pImpl->m_TexturesDirty = true;
	m_pImpl->m_FirstDirtyTexture = Math::Min(m_pImpl->m_FirstDirtyTexture, (int)slot);
	m_pImpl->m_LastDirtyTexture = Math::Max(m_pImpl->m_LastDirtyTexture, (int)slot);
}


void Graphics::Draw(const PrimitiveType type, const int vertexStart, const int vertexCount)
{
	AUTOPROFILE(Graphics_Draw);
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
	AUTOPROFILE(Graphics_DrawIndexed);
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
	AUTOPROFILE(Graphics_DrawIndexedInstanced);
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
	AUTOPROFILE(Graphics_Clear);

	if (m_CurrentViewport.Left == 0 && m_CurrentViewport.Top == 0 && m_CurrentViewport.Right == m_WindowWidth && m_CurrentViewport.Bottom == m_WindowHeight)
	{
		PrepareDraw();
		ID3D11RenderTargetView* pRtv = m_pImpl->m_RenderTargetViews[0];
		ID3D11DepthStencilView* pDsv = m_pImpl->m_pDepthStencilView;
		if (pRtv && (clearFlags & ClearFlags::RenderTarget) == ClearFlags::RenderTarget)
		{
			m_pImpl->GetDeviceContext()->ClearRenderTargetView(pRtv, &color.x);
		}
		if (pDsv)
		{
			unsigned int depthClearFlags = 0;
			if ((clearFlags & ClearFlags::Depth) == ClearFlags::Depth)
			{
				depthClearFlags |= D3D11_CLEAR_DEPTH;
			}
			if ((clearFlags & ClearFlags::Stencil) == ClearFlags::Stencil)
			{
				depthClearFlags |= D3D11_CLEAR_STENCIL;
			}
			if (depthClearFlags != 0)
			{
				m_pImpl->m_pDeviceContext->ClearDepthStencilView(pDsv, depthClearFlags, depth, stencil);
			}
		}
	}
	else
	{
		GetDepthStencilState()->SetDepthTest(CompareMode::ALWAYS);
		GetDepthStencilState()->SetDepthWrite((clearFlags & ClearFlags::Depth) == ClearFlags::Depth);
		GetBlendState()->SetColorWrite(((clearFlags & ClearFlags::RenderTarget) == ClearFlags::RenderTarget) ? ColorWrite::ALL : ColorWrite::NONE);
		GetDepthStencilState()->SetStencilTest((clearFlags & ClearFlags::Stencil) == ClearFlags::Stencil, CompareMode::ALWAYS, StencilOperation::REF, StencilOperation::KEEP, StencilOperation::KEEP, stencil, 0XFF, 0XFF);

		Geometry* quadGeometry = GetSubsystem<Renderer>()->GetQuadGeometry();

		InvalidateShaders();
		SetShader(ShaderType::VertexShader, GetShader("Shaders/ClearFrameBuffer", ShaderType::VertexShader));
		SetShader(ShaderType::PixelShader, GetShader("Shaders/ClearFrameBuffer", ShaderType::PixelShader));

		Matrix worldMatrix = Matrix::CreateTranslation(Vector3(0, 0, depth));

		SetShaderParameter(ShaderConstant::cColor, color);
		SetShaderParameter(ShaderConstant::cWorld, worldMatrix);

		quadGeometry->Draw(this);
	}
}

void Graphics::FlushRenderTargetChanges(bool force)
{
	if (m_pImpl->m_RenderTargetsDirty || force)
	{
		for (int i = 0; i < GraphicsConstants::MAX_RENDERTARGETS; ++i)
		{
			m_pImpl->m_RenderTargetViews[i] = m_CurrentRenderTargets[i] ? (ID3D11RenderTargetView*)m_CurrentRenderTargets[i]->GetRenderTargetView() : nullptr;
		}

		if (m_pImpl->m_RenderTargetViews[0] == nullptr && m_RenderDepthOnly == false)
		{
			m_pImpl->m_RenderTargetViews[0] = (ID3D11RenderTargetView*)m_pDefaultRenderTarget->GetRenderTarget()->GetRenderTargetView();
		}

		m_pImpl->m_pDepthStencilView = m_pCurrentDepthStencil ? (ID3D11DepthStencilView*)m_pCurrentDepthStencil->GetRenderTargetView() : (ID3D11DepthStencilView*)m_pDefaultDepthStencil->GetRenderTarget()->GetRenderTargetView();
		m_pImpl->m_pDeviceContext->OMSetRenderTargets(GraphicsConstants::MAX_RENDERTARGETS, m_pImpl->m_RenderTargetViews.data(), m_pImpl->m_pDepthStencilView);
		m_pImpl->m_RenderTargetsDirty = false;
	}
}

void Graphics::FlushSRVChanges(bool force)
{
	if ((m_pImpl->m_TexturesDirty || force) && m_pImpl->m_LastDirtyTexture - m_pImpl->m_FirstDirtyTexture + 1 > 0)
	{
		if (m_CurrentShaders[(int)ShaderType::VertexShader])
		{
			m_pImpl->m_pDeviceContext->VSSetShaderResources(m_pImpl->m_FirstDirtyTexture, m_pImpl->m_LastDirtyTexture - m_pImpl->m_FirstDirtyTexture + 1, m_pImpl->m_ShaderResourceViews.data() + m_pImpl->m_FirstDirtyTexture);
			m_pImpl->m_pDeviceContext->VSSetSamplers(m_pImpl->m_FirstDirtyTexture, m_pImpl->m_LastDirtyTexture - m_pImpl->m_FirstDirtyTexture + 1, m_pImpl->m_SamplerStates.data() + m_pImpl->m_FirstDirtyTexture);
		}
		if (m_CurrentShaders[(int)ShaderType::PixelShader])
		{
			m_pImpl->m_pDeviceContext->PSSetShaderResources(m_pImpl->m_FirstDirtyTexture, m_pImpl->m_LastDirtyTexture - m_pImpl->m_FirstDirtyTexture + 1, m_pImpl->m_ShaderResourceViews.data() + m_pImpl->m_FirstDirtyTexture);
			m_pImpl->m_pDeviceContext->PSSetSamplers(m_pImpl->m_FirstDirtyTexture, m_pImpl->m_LastDirtyTexture - m_pImpl->m_FirstDirtyTexture + 1, m_pImpl->m_SamplerStates.data() + m_pImpl->m_FirstDirtyTexture);
		}
#ifdef SHADER_TESSELLATION_ENABLE
		if (m_CurrentShaders[(int)ShaderType::DomainShader])
		{
			m_pImpl->m_pDeviceContext->DSSetShaderResources(m_pImpl->m_FirstDirtyTexture, m_pImpl->m_LastDirtyTexture - m_pImpl->m_FirstDirtyTexture + 1, m_pImpl->m_ShaderResourceViews.data() + m_pImpl->m_FirstDirtyTexture);
			m_pImpl->m_pDeviceContext->DSSetSamplers(m_pImpl->m_FirstDirtyTexture, m_pImpl->m_LastDirtyTexture - m_pImpl->m_FirstDirtyTexture + 1, m_pImpl->m_SamplerStates.data() + m_pImpl->m_FirstDirtyTexture);
		}
#endif

		m_pImpl->m_TexturesDirty = false;
		m_pImpl->m_FirstDirtyTexture = (int)TextureSlot::MAX;
		m_pImpl->m_LastDirtyTexture = 0;
	}
}


void Graphics::PrepareDraw()
{
	AUTOPROFILE(Graphics_PrepareDraw);

	FlushRenderTargetChanges(false);
	FlushSRVChanges(false);

	UpdateShaders();

	if (m_DepthStencilState.IsDirty())
	{
		AUTOPROFILE(Graphics_PrepareDraw_SetDepthStencilState);
		ID3D11DepthStencilState* pState = (ID3D11DepthStencilState*)m_DepthStencilState.GetOrCreate(this);
		m_pImpl->m_pDeviceContext->OMSetDepthStencilState(pState, m_DepthStencilState.GetStencilRef());
	}

	if (m_RasterizerState.IsDirty())
	{
		AUTOPROFILE(Graphics_PrepareDraw_SetRasterizerState);
		ID3D11RasterizerState* pState = (ID3D11RasterizerState*)m_RasterizerState.GetOrCreate(this);
		m_pImpl->m_pDeviceContext->RSSetState(pState);
	}

	if (m_BlendState.IsDirty())
	{
		AUTOPROFILE(Graphics_PrepareDraw_SetBlendState);
		ID3D11BlendState* pBlendState = (ID3D11BlendState*)m_BlendState.GetOrCreate(this);
		m_pImpl->m_pDeviceContext->OMSetBlendState(pBlendState, nullptr, UINT_MAX);
	}

	if (m_pImpl->m_VertexBuffersDirty)
	{
		AUTOPROFILE(Graphics_PrepareDraw_SetVertexBuffers);

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

		if (hash == 0)
		{
			m_pImpl->m_pDeviceContext->IASetInputLayout(nullptr);
		}
		else
		{
			auto pInputLayout = m_pImpl->m_InputLayoutMap.find(hash);
			if (pInputLayout != m_pImpl->m_InputLayoutMap.end())
			{
				m_pImpl->m_pDeviceContext->IASetInputLayout((ID3D11InputLayout*)pInputLayout->second->GetResource());
			}
			else
			{
				std::unique_ptr<InputLayout> pNewInputLayout = std::make_unique<InputLayout>(this);
				pNewInputLayout->Create(m_CurrentVertexBuffers.data(), (unsigned int)m_CurrentVertexBuffers.size(), m_CurrentShaders[(unsigned int)ShaderType::VertexShader]);
				m_pImpl->m_pDeviceContext->IASetInputLayout((ID3D11InputLayout*)pNewInputLayout->GetResource());
				m_pImpl->m_InputLayoutMap[hash] = std::move(pNewInputLayout);
			}
		}

		m_pImpl->m_FirstDirtyVertexBuffer = UINT_MAX;
		m_pImpl->m_LastDirtyVertexBuffer = 0;
		m_pImpl->m_VertexBuffersDirty = false;
	}

	if (m_ScissorRectDirty)
	{
		AUTOPROFILE(Graphics_PrepareDraw_SetScissorRect);

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
		{
			continue;
		}
		for (ConstantBuffer* pBuffer : pShader->GetConstantBuffers())
		{
			if (pBuffer)
			{
				pBuffer->Apply();
			}
		}
	}
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
	m_pImpl->m_pSwapChain->Present(m_Vsync ? 1 : 0, 0);

	PROFILER_EVENT(Graphics_EndFrame);
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
	swapDesc.OutputWindow = GetWindow();
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = m_WindowType != WindowType::FULLSCREEN;

	//Create the swap chain
	HR(m_pImpl->m_pFactory->CreateSwapChain(m_pImpl->m_pDevice.Get(), &swapDesc, m_pImpl->m_pSwapChain.GetAddressOf()));

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

	m_pDefaultRenderTarget.reset();
	m_pDefaultDepthStencil.reset();

	ID3D11RenderTargetView* emptyView = nullptr;
	m_pImpl->m_pDeviceContext->OMSetRenderTargets(1, &emptyView, nullptr);

	HR(m_pImpl->m_pSwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	ID3D11Texture2D *pBackbuffer = nullptr;
	HR(m_pImpl->m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackbuffer)));

	m_pDefaultRenderTarget = std::make_unique<Texture2D>(m_pContext);
	m_pDefaultRenderTarget->SetSize(width, height, DXGI_FORMAT_R8G8B8A8_UNORM, TextureUsage::RENDERTARGET, m_Multisample, pBackbuffer);
	m_pDefaultDepthStencil = std::make_unique<Texture2D>(m_pContext);
	m_pDefaultDepthStencil->SetSize(width, height, DXGI_FORMAT_R24G8_TYPELESS, TextureUsage::DEPTHSTENCILBUFFER, m_Multisample, nullptr);

	for (int i = 0; i < GraphicsConstants::MAX_RENDERTARGETS; ++i)
	{
		SetRenderTarget(i, nullptr);
	}
	SetDepthStencil(nullptr);
	m_CurrentViewport.Left = 0;
	m_CurrentViewport.Top = 0;
	m_CurrentViewport.Right = (float)width;
	m_CurrentViewport.Bottom = (float)height;
	SetViewport(m_CurrentViewport);

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
	AUTOPROFILE(Graphics_TakeScreenshot);

	D3D11_TEXTURE2D_DESC desc = {};
	desc.ArraySize = 1;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Width = m_WindowWidth;
	desc.Height = m_WindowHeight;
	desc.MipLevels = 1;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_STAGING;

	ComPtr<ID3D11Texture2D> pStagingTexture;
	HR(m_pImpl->GetDevice()->CreateTexture2D(&desc, nullptr, pStagingTexture.GetAddressOf()));

	ID3D11Resource* pRenderTexture = (ID3D11Resource*)m_pDefaultRenderTarget->GetResource();

	//If we are using MSAA, we need to resolve the resource first
	if (m_Multisample > 1)
	{
		ComPtr<ID3D11Texture2D> pResolveTexture;

		D3D11_TEXTURE2D_DESC resolveTexDesc = {};
		resolveTexDesc.ArraySize = 1;
		resolveTexDesc.CPUAccessFlags = 0;
		resolveTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		resolveTexDesc.Width = m_WindowWidth;
		resolveTexDesc.Height = m_WindowHeight;
		resolveTexDesc.MipLevels = 1;
		resolveTexDesc.MiscFlags = 0;
		resolveTexDesc.SampleDesc.Count = 1;
		resolveTexDesc.SampleDesc.Quality = 0;
		resolveTexDesc.Usage = D3D11_USAGE_DEFAULT;

		HR(m_pImpl->GetDevice()->CreateTexture2D(&resolveTexDesc, nullptr, pResolveTexture.GetAddressOf()));

		m_pImpl->GetDeviceContext()->ResolveSubresource(pResolveTexture.Get(), 0, (ID3D11Texture2D*)pRenderTexture, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		m_pImpl->GetDeviceContext()->CopyResource(pStagingTexture.Get(), pResolveTexture.Get());
	}
	else
	{
		m_pImpl->GetDeviceContext()->CopyResource(pStagingTexture.Get(), (ID3D11Texture2D*)pRenderTexture);
	}

	Image destinationImage(m_pContext);
	destinationImage.SetSize(m_WindowWidth, m_WindowHeight, 3);

	D3D11_MAPPED_SUBRESOURCE pData = {};
	m_pImpl->GetDeviceContext()->Map(pStagingTexture.Get(), 0, D3D11_MAP_READ, 0, &pData);
	unsigned char* pDest = destinationImage.GetWritableData();
	for (int y = 0; y < m_WindowHeight; ++y)
	{
		unsigned char* pSrc = (unsigned char*)pData.pData + y * pData.RowPitch;
		for (int x = 0; x < m_WindowWidth; ++x)
		{
			*pDest++ = *pSrc++;
			*pDest++ = *pSrc++;
			*pDest++ = *pSrc++;
			++pSrc;
		}
	}
	m_pImpl->GetDeviceContext()->Unmap(pStagingTexture.Get(), 0);

	destinationImage.Save(outputStream);
}

ConstantBuffer* Graphics::GetOrCreateConstantBuffer(unsigned int index, unsigned int size)
{
	size_t hash = (unsigned)(index << 1)
		| (unsigned)(size << 4);

	auto pIt = m_ConstantBuffers.find(hash);
	if (pIt != m_ConstantBuffers.end())
	{
		return pIt->second.get();
	}
	std::unique_ptr<ConstantBuffer> pBuffer = std::make_unique<ConstantBuffer>(this);
	pBuffer->SetSize(size);
	m_ConstantBuffers[hash] = std::move(pBuffer);
	return m_ConstantBuffers[hash].get();
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