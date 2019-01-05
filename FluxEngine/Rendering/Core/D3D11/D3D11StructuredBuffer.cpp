#include "FluxEngine.h"
#include "../StructuredBuffer.h"
#include "D3D11GraphicsImpl.h"
#include "../Graphics.h"

void StructuredBuffer::Create(const int elementCount, const int elementStride, bool dynamic /*= false*/)
{
	Release();

	m_ElementCount = elementCount;
	m_Stride = elementStride;

	D3D11_BUFFER_DESC desc;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = elementStride * elementCount;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = elementStride;
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateBuffer(&desc, nullptr, (ID3D11Buffer**)&m_pResource));

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
	viewDesc.Buffer.FirstElement = 0;
	viewDesc.Buffer.NumElements = elementCount;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateShaderResourceView((ID3D11Buffer*)m_pResource, &viewDesc, (ID3D11ShaderResourceView**)&m_pShaderResourceView));
}

void StructuredBuffer::SetData(void* pData)
{
	void* pTarget = Map(true);
	memcpy(pTarget, pData, m_ElementCount * m_Stride);
	Unmap();
}

void* StructuredBuffer::Map(bool discard)
{
	assert(m_pResource);
	if (m_Mapped == false)
	{
		D3D11_MAPPED_SUBRESOURCE data;
		HR(m_pGraphics->GetImpl()->GetDeviceContext()->Map((ID3D11Buffer*)m_pResource, 0, discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE, 0, &data));
		m_pMappedData = data.pData;
		m_Mapped = true;
	}
	return m_pMappedData;
}

void StructuredBuffer::Unmap()
{
	if (m_Mapped)
	{
		m_pGraphics->GetImpl()->GetDeviceContext()->Unmap((ID3D11Buffer*)m_pResource, 0);
		m_Mapped = false;
	}
}

void StructuredBuffer::Release()
{
	SafeRelease(m_pResource);
	SafeRelease(m_pShaderResourceView);
}
