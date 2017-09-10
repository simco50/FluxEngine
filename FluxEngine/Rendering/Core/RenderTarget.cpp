#include "stdafx.h"
#include "RenderTarget.h"


RenderTarget::RenderTarget(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext) :
	m_pD3Device(pDevice), m_pD3DeviceContext(pDeviceContext)
{
}

RenderTarget::~RenderTarget()
{
	Reset();
}

void RenderTarget::Create(const RENDER_TARGET_DESC& renderTargetDesc)
{
	Reset();
	m_RenderTargetDesc = renderTargetDesc;
	if (m_RenderTargetDesc.ColorBuffer)
		CreateColor();
	if (m_RenderTargetDesc.DepthBuffer)
		CreateDepth();
}

void RenderTarget::Reset()
{
	//Reset everything so we can recreate the RenderTarget if needed
	m_pRenderTargetView.Reset();
	m_pColorSRV.Reset();
	m_pColorBuffer.Reset();

	m_pDepthStencilView.Reset();
	m_pDepthSRV.Reset();
	m_pDepthBuffer.Reset();
}

void RenderTarget::CreateColor()
{
	//BUFFER
	//Use the provided resource if there is one
	if(m_RenderTargetDesc.pColor)
		m_pColorBuffer = m_RenderTargetDesc.pColor;
	else
	{
		D3D11_TEXTURE2D_DESC bufferDesc = {};
		bufferDesc.Width = m_RenderTargetDesc.Width;
		bufferDesc.Height = m_RenderTargetDesc.Height;
		bufferDesc.ArraySize = 1;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.Format = m_RenderTargetDesc.ColorFormat;
		bufferDesc.MipLevels = 1;
		if (m_RenderTargetDesc.MSAA)
		{
			bufferDesc.SampleDesc.Count = 4;
			bufferDesc.SampleDesc.Quality = m_RenderTargetDesc.MsaaQuality - 1;
		}
		else
		{
			bufferDesc.SampleDesc.Count = 1;
			bufferDesc.SampleDesc.Quality = 0;
		}
		bufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
		if(m_RenderTargetDesc.ColorSRV)
			bufferDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;

		HR(m_pD3Device->CreateTexture2D(&bufferDesc, nullptr, m_pColorBuffer.GetAddressOf()));
	}

	//RTV
	HR(m_pD3Device->CreateRenderTargetView(m_pColorBuffer.Get(), nullptr, m_pRenderTargetView.GetAddressOf()));

	//SRV
	if (m_RenderTargetDesc.ColorSRV)
		HR(m_pD3Device->CreateShaderResourceView(m_pColorBuffer.Get(), nullptr, m_pColorSRV.GetAddressOf()));
}

void RenderTarget::CreateDepth()
{
	//BUFFER
	//Use provided resource if there is one
	if (m_RenderTargetDesc.pDepth)
		m_pDepthBuffer = m_RenderTargetDesc.pDepth;
	else
	{
		D3D11_TEXTURE2D_DESC depthStencilDesc = {};
		depthStencilDesc.Width = m_RenderTargetDesc.Width;
		depthStencilDesc.Height = m_RenderTargetDesc.Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = GetDepthResourceFormat(m_RenderTargetDesc.DepthFormat);
		if (m_RenderTargetDesc.MSAA)
		{
			depthStencilDesc.SampleDesc.Count = 4;
			depthStencilDesc.SampleDesc.Quality = m_RenderTargetDesc.MsaaQuality - 1;
		}
		else
		{
			depthStencilDesc.SampleDesc.Count = 1;
			depthStencilDesc.SampleDesc.Quality = 0;
		}
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		if (m_RenderTargetDesc.DepthSRV)
			depthStencilDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		HR(m_pD3Device->CreateTexture2D(&depthStencilDesc, nullptr, m_pDepthBuffer.GetAddressOf()));
	}

	//DSV
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = m_RenderTargetDesc.DepthFormat;
	if (m_RenderTargetDesc.MSAA)
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	else
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = 0;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(m_pD3Device->CreateDepthStencilView(m_pDepthBuffer.Get(), &dsvDesc, m_pDepthStencilView.GetAddressOf()));

	if(m_RenderTargetDesc.DepthSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = GetDepthSRVFormat(m_RenderTargetDesc.DepthFormat);
		if(m_RenderTargetDesc.MSAA)
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		else
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		HR(m_pD3Device->CreateShaderResourceView(m_pDepthBuffer.Get(), &srvDesc, m_pDepthSRV.GetAddressOf()));
	}
}

DXGI_FORMAT RenderTarget::GetDepthResourceFormat(DXGI_FORMAT initFormat)
{
	DXGI_FORMAT resourceFormat = {};
	switch (initFormat)
	{
	case DXGI_FORMAT::DXGI_FORMAT_D16_UNORM:
		resourceFormat = DXGI_FORMAT::DXGI_FORMAT_R16_TYPELESS;
		break;
	case DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT:
		resourceFormat = DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;
		break;
	case DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT:
		resourceFormat = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
		break;
	case DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		resourceFormat = DXGI_FORMAT::DXGI_FORMAT_R32G8X24_TYPELESS;
		break;
	default:
		Console::Log("RenderTarget::GetDepthSRVFormat() -> Format not supported!", LogType::ERROR);
		break;
	}

	return resourceFormat;
}

DXGI_FORMAT RenderTarget::GetDepthSRVFormat(DXGI_FORMAT initFormat)
{
	DXGI_FORMAT srvFormat = {};
	switch (initFormat)
	{
	case DXGI_FORMAT::DXGI_FORMAT_D16_UNORM:
		srvFormat = DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT;
		break;
	case DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT:
		srvFormat = DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		break;
	case DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT:
		srvFormat = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
		break;
	case DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		srvFormat = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		break;
	default:
		Console::Log("RenderTarget::GetDepthSRVFormat() -> Format not supported!", LogType::ERROR);
		break;
	}

	return srvFormat;
}

void RenderTarget::ClearColor()
{
	if (m_pRenderTargetView)
		m_pD3DeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<float*>(&m_RenderTargetDesc.ClearColor));
	else
		Console::Log("RenderTarget::ClearColor() -> RenderTarget does not have a RenderTargetView", LogType::ERROR);
}

void RenderTarget::ClearDepth()
{
	if (m_pRenderTargetView)
		m_pD3DeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	else
		Console::Log("RenderTarget::ClearColor() -> RenderTarget does not have a DepthStencilView", LogType::ERROR);
}

ID3D11ShaderResourceView* RenderTarget::GetColorSRV() const
{
	if (m_pColorSRV)
		return m_pColorSRV.Get();
	Console::Log("RenderTarget::ClearColor() -> RenderTarget does not have a ColorShaderResourceView", LogType::ERROR);
	return nullptr;
}

ID3D11ShaderResourceView* RenderTarget::GetDepthSRV() const
{
	if (m_pDepthSRV)
		return m_pDepthSRV.Get();
	Console::Log("RenderTarget::ClearColor() -> RenderTarget does not have a DepthShaderResourceView", LogType::ERROR);
	return nullptr;
}