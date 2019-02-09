#pragma once

#include <dxgi.h>
#include <d3d11.h>

class GraphicsImpl
{
public:
	friend class Graphics;
	friend class PipelineState;
	friend class GraphicsPipelineState;
	friend class CommandContext;
	friend class GraphicsCommandContext;
	friend class ComputeCommandContext;

	ID3D11Device* GetDevice() const { return m_pDevice.Get(); }
	ComPtr<ID3D11Device>& GetDeviceCom() { return m_pDevice; }
	ID3D11DeviceContext* GetDeviceContext() const { return m_pDeviceContext.Get(); }

	unsigned int GetMultisampleQuality(DXGI_FORMAT format, unsigned int sampleCount) const;
	bool CheckMultisampleQuality(DXGI_FORMAT format, unsigned int sampleCount) const;

private:
	static bool GetPrimitiveType(PrimitiveType primitiveType, unsigned int elementCount, D3D11_PRIMITIVE_TOPOLOGY& type, unsigned int& primitiveCount);

	ComPtr<IDXGIAdapter> m_pAdapter;
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	ComPtr<IDXGIFactory> m_pFactory;
	ComPtr<IDXGISwapChain> m_pSwapChain;

	//Textures
	bool m_TexturesDirty = false;
	int m_FirstDirtyTexture = (int)TextureSlot::MAX;
	int m_LastDirtyTexture = 0;
	std::array<ID3D11SamplerState*, (size_t)TextureSlot::MAX> m_SamplerStates = {};
	std::array<ID3D11ShaderResourceView*, (size_t)TextureSlot::MAX> m_ShaderResourceViews = {};

	//InputLayouts
	std::map<unsigned long long, ComPtr<ID3D11InputLayout>> m_InputLayoutMap;

	//Render Target
	std::array<ID3D11RenderTargetView*, GraphicsConstants::MAX_RENDERTARGETS> m_RenderTargetViews = {};
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;

	bool m_RenderTargetsDirty = true;

	//Primitive topology
	D3D11_PRIMITIVE_TOPOLOGY m_CurrentPrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

	//Vertex buffer
	std::array<ID3D11Buffer*, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentVertexBuffers = {};
	std::array<unsigned int, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentOffsets = {};
	std::array<unsigned int, GraphicsConstants::MAX_VERTEX_BUFFERS> m_CurrentStrides = {};
	unsigned int m_FirstDirtyVertexBuffer = UINT_MAX;
	unsigned int m_LastDirtyVertexBuffer = 0;
	bool m_VertexBuffersDirty = false;
	bool m_InputLayoutDirty = false;

	//Pipeline state
	bool m_RasterizerStateDirty = false;
	bool m_DepthStencilStateDirty = false;
	bool m_BlendStateDirty = false;
	std::map<unsigned int, ComPtr<ID3D11DepthStencilState>> m_DepthStencilStates;
	std::map<unsigned int, ComPtr<ID3D11RasterizerState>> m_RasterizerStates;
	std::map<unsigned int, ComPtr<ID3D11BlendState>> m_BlendStates;
};
