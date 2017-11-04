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
	static inline bool GetPrimitiveType(const PrimitiveType primitiveType, const unsigned int elementCount, D3D11_PRIMITIVE_TOPOLOGY& type, unsigned int& primitiveCount);

	ComPtr<IDXGIAdapter> m_pAdapter;
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	ComPtr<IDXGIFactory> m_pFactory;
	ComPtr<IDXGISwapChain> m_pSwapChain;

	vector<ID3D11SamplerState*> m_CurrentSamplerStates;
	vector<ID3D11ShaderResourceView*> m_CurrentShaderResourceViews;

	map<unsigned long long, unique_ptr<InputLayout>> m_InputLayoutMap;

	D3D11_PRIMITIVE_TOPOLOGY m_CurrentPrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

	array<ID3D11Buffer*, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentVertexBuffers;
	array<unsigned int, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentOffsets;
	array<unsigned int, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentStrides;
};

inline bool GraphicsImpl::GetPrimitiveType(const PrimitiveType primitiveType, const unsigned int elementCount, D3D11_PRIMITIVE_TOPOLOGY& type, unsigned int& primitiveCount)
{
	switch (primitiveType)
	{
	case PrimitiveType::TRIANGLELIST:
		type = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		primitiveCount = elementCount / 3;
		return true;
	case PrimitiveType::POINTLIST:
		type = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		primitiveCount = elementCount / 2;
		return true;
	case PrimitiveType::TRIANGLESTRIP:
		type = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		primitiveCount = elementCount - 2;
		return true;
	case PrimitiveType::UNDEFINED:
	default:
		FLUX_LOG(ERROR, "[Graphics::SetPrimitiveType()] > Invalid primitive type");
		return false;
	}
}
