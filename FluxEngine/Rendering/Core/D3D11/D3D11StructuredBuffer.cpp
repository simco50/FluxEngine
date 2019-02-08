#include "FluxEngine.h"
#include "../StructuredBuffer.h"
#include "D3D11GraphicsImpl.h"
#include "../Graphics.h"

void StructuredBuffer::Create(const int elementCount, const int elementStride, bool dynamic /*= false*/)
{
	Release();

	m_ElementCount = elementCount;
	m_ElementStride = elementStride;
	m_Size = elementStride * elementCount;

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