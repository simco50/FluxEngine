#include "D3D11GraphicsImpl.h"

void ConstantBuffer::SetSize(const unsigned int size)
{
	AUTOPROFILE(CreateConstantBuffer);

	Release();
	m_Size = size;

	m_pShadowData = new unsigned char[m_Size];

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = m_Size;
	desc.CPUAccessFlags = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateBuffer(&desc, nullptr, (ID3D11Buffer**)&m_pBuffer));
}

void ConstantBuffer::Apply()
{
	if (m_IsDirty && m_pBuffer)
	{
		m_pGraphics->GetImpl()->GetDeviceContext()->UpdateSubresource((ID3D11Buffer*)m_pBuffer, 0, 0, (void*)m_pShadowData, 0, 0);
		m_IsDirty = false;
	}
}