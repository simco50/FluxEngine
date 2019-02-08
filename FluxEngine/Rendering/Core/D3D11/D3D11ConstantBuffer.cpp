#include "FluxEngine.h"
#include "D3D11GraphicsImpl.h"
#include "../ConstantBuffer.h"
#include "../Graphics.h"

void ConstantBuffer::SetSize(const unsigned int size)
{
	AUTOPROFILE(ConstantBuffer_SetSize);

	Release();
	m_Size = size;

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = m_Size;
	desc.CPUAccessFlags = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateBuffer(&desc, nullptr, (ID3D11Buffer**)&m_pResource));
}