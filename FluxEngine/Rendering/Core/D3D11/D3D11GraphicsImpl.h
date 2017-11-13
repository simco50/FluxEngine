#pragma once
#include "Rendering/Core/GraphicsDefines.h"
#include "Rendering/Core/D3DCommon/D3DHelpers.h"

class InputLayout;

class GraphicsImpl
{
public:
	friend class Graphics;

	ID3D11Device* GetDevice() const { return m_pDevice.Get(); }
	ID3D11DeviceContext* GetDeviceContext() const { return m_pDeviceContext.Get(); }

	unsigned int GetMultisampleQuality(const DXGI_FORMAT format, const unsigned int sampleCount) const;
	bool CheckMultisampleQuality(const DXGI_FORMAT format, const unsigned int sampleCount) const;

private:
	static bool GetPrimitiveType(const PrimitiveType primitiveType, const unsigned int elementCount, D3D11_PRIMITIVE_TOPOLOGY& type, unsigned int& primitiveCount);

	ComPtr<IDXGIAdapter> m_pAdapter;
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	ComPtr<IDXGIFactory> m_pFactory;
	ComPtr<IDXGISwapChain> m_pSwapChain;

	/////Resource cache/////

	vector<ID3D11SamplerState*> m_CurrentSamplerStates;
	vector<ID3D11ShaderResourceView*> m_CurrentShaderResourceViews;

	ComPtr<ID3D11Texture2D> m_pBackbufferResolveTexture;

	map<unsigned long long, unique_ptr<InputLayout>> m_InputLayoutMap;

	D3D11_PRIMITIVE_TOPOLOGY m_CurrentPrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

	//Vertex buffer
	array<ID3D11Buffer*, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentVertexBuffers;
	array<unsigned int, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentOffsets;
	array<unsigned int, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentStrides;
};
