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
		m_pGraphics->GetImpl()->GetDevice()->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)name.size(), name.data());
	}
}

#else

void GraphicsObject::SetName(const std::string& name)
{

}

#endif