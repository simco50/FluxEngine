#include "FluxEngine.h"
#include "GraphicsObject.h"
#include "Graphics.h"

GraphicsObject::GraphicsObject(Graphics* pGraphics)
	: m_pGraphics(pGraphics)
{

}

#ifdef GRAPHICS_D3D11
#include "D3D11/D3D11GraphicsImpl.h"

void GraphicsObject::SetName(const std::string& name)
{
	if (m_pResource)
	{
		static_cast<ID3D11Resource*>(m_pResource)->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)name.size(), name.data());
	}
}

#elif defined(GRAPHICS_D3D12)
#include "D3D12/D3D12GraphicsImpl.h"

void GraphicsObject::SetName(const std::string& name)
{
	if (m_pResource)
	{
		std::wstring n(name.begin(), name.end());
		static_cast<ID3D12Resource*>(m_pResource)->SetName(n.c_str());
	}
}

#else

void GraphicsObject::SetName(const std::string& /*name*/)
{

}

#endif