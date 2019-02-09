#include "FluxEngine.h"
#include "GraphicsResource.h"

GraphicsResource::GraphicsResource(Graphics* pGraphics) : GraphicsObject(pGraphics)
{

}

GraphicsResource::~GraphicsResource()
{
	Release();
}

bool GraphicsResource::SetData(const void* pData, int offset, int size, bool immediate)
{
	checkf(size + offset <= m_Size, "[GraphicsResource::SetData] Trying to write data out of bounds");
	check(m_pResource);

	AUTOPROFILE(GraphicsResource_SetData);

	if (immediate)
	{
		return Apply_Internal(pData, offset, size);
	}
	else
	{
		if (m_ShadowData.GetSize() != m_Size)
		{
			m_ShadowData.SetSize(m_Size);
		}
		m_ShadowData.SetData(pData, offset, size);
		m_IsDirty = true;
	}
	return true;
}

bool GraphicsResource::SetData(const void* pData, bool immediate)
{
	return SetData(pData, 0, m_Size, immediate);
}

bool GraphicsResource::SetElement(int index, const void* pData, int size, bool immediate)
{
	checkf(size == m_ElementStride, "[GraphicsResource::SetElement] > Element size should be the same as the stride");
	return SetData(pData, index * m_ElementStride, size, immediate);
}

void GraphicsResource::Apply()
{
	if (m_IsDirty)
	{
		Apply_Internal(m_ShadowData.GetData(), 0, m_Size);
		m_IsDirty = false;
	}
}