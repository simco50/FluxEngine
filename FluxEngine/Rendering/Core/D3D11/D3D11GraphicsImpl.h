#pragma once
#include "..\GraphicsDefines.h"

class InputLayout;

class GraphicsImpl
{
public:
	friend class Graphics;

	ID3D11Device* GetDevice() const { return m_pDevice.Get(); }
	ID3D11DeviceContext* GetDeviceContext() const { return m_pDeviceContext.Get(); }

private:
	ComPtr<IDXGIAdapter> m_pAdapter;
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	ComPtr<IDXGIFactory> m_pFactory;
	ComPtr<IDXGISwapChain> m_pSwapChain;

	vector<ID3D11SamplerState*> m_CurrentSamplerStates;
	vector<ID3D11ShaderResourceView*> m_CurrentShaderResourceViews;

	map<unsigned long long, unique_ptr<InputLayout>> m_InputLayoutMap;

	array<ID3D11Buffer*, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentVertexBuffers;
	array<unsigned int, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentOffsets;
	array<unsigned int, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentStrides;
};