#pragma once

#include <dxgi.h>
#include <d3d12.h>
#include "d3dx12.h"

class ShaderProgram;

class GraphicsImpl
{
public:
	friend class Graphics;

	ID3D12Device* GetDevice() const { return m_pDevice.Get(); }
	ComPtr<ID3D12Device>& GetDeviceCom() { return m_pDevice; }

	unsigned int GetMultisampleQuality(DXGI_FORMAT format, unsigned int sampleCount) const;
	bool CheckMultisampleQuality(DXGI_FORMAT format, unsigned int sampleCount) const;

	ID3D12GraphicsCommandList* GetTemporaryCommandList();

	D3D12_CPU_DESCRIPTOR_HANDLE GetDsv() { return m_pDsvHeap->GetCPUDescriptorHandleForHeapStart(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetRtv(int frameIndex) { return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pRtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, m_RtvDescriptorSize); }

private:
	static bool GetPrimitiveType(PrimitiveType primitiveType, unsigned int elementCount, D3D12_PRIMITIVE_TOPOLOGY& type, unsigned int& primitiveCount);
	static const int FRAME_COUNT = 2;

	ComPtr<IDXGIAdapter> m_pAdapter;
	ComPtr<ID3D12Device> m_pDevice;
	ComPtr<IDXGIFactory2> m_pFactory;
	ComPtr<IDXGISwapChain> m_pSwapChain;
	ComPtr<ID3D12CommandQueue> m_pCommandQueue;
	ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_pDsvHeap;
	std::array<ComPtr<ID3D12Resource>, FRAME_COUNT> m_RenderTargets;
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer;

	int m_CurrentBackBufferIndex = 0;

	int m_RtvDescriptorSize = -1;
	int m_DsvDescriptorSize = -1;
	int m_CbvSrvDescriptorSize = -1;

	//Shader programs
	ShaderProgram* m_pCurrentShaderProgram = nullptr;
	std::map<uint64, std::unique_ptr<ShaderProgram>> m_ShaderPrograms;
	bool m_ShaderProgramDirty = false;
};
