#include "FluxEngine.h"
#include "../GraphicsResource.h"
#include "../Graphics.h"
#include "D3D11GraphicsImpl.h"

bool GraphicsResource::Apply_Internal(const void* pData, int offset, int size)
{
	if (m_Dynamic)
	{
		void* pTarget = Map(true);
		memcpy(static_cast<char*>(pTarget) + offset, pData, size);
		Unmap();
	}
	else
	{
		D3D11_BOX destBox;
		destBox.left = offset;
		destBox.right = offset + size;
		destBox.top = 0;
		destBox.bottom = 1;
		destBox.front = 0;
		destBox.back = 1;

		D3D11_BOX* pDest = nullptr;
		if (size != m_Size)
		{
			pDest = &destBox;
		}

		m_pGraphics->GetImpl()->GetDeviceContext()->UpdateSubresource((ID3D11Buffer*)m_pResource, 0, pDest, pData, 0, 0);
	}
	return true;
}


void* GraphicsResource::Map(bool discard)
{
	checkf(m_Dynamic, "[GraphicsResource::Map] Resource must be dynamic");
	check(m_pResource);
	if (m_Mapped == false)
	{
		D3D11_MAPPED_SUBRESOURCE data;
		HR(m_pGraphics->GetImpl()->GetDeviceContext()->Map((ID3D11Buffer*)m_pResource, 0, discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE, 0, &data));
		m_pMappedData = data.pData;
		m_Mapped = true;
	}
	return m_pMappedData;
}

void GraphicsResource::Unmap()
{
	if (m_Mapped)
	{
		m_pGraphics->GetImpl()->GetDeviceContext()->Unmap((ID3D11Buffer*)m_pResource, 0);
		m_Mapped = false;
	}
}

void GraphicsResource::Release()
{
	SafeRelease(m_pResource);
	SafeRelease(m_pShaderResourceView);
}