#pragma once

class Graphics;

#ifdef _DEBUG
#define GRAPHICS_SET_NAME(name) SetName(name);
#else
#define GRAPHICS_SET_NAME(name)
#endif

#if defined(GRAPHICS_D3D11) || defined(GRAPHICS_D3D12)
using GraphicsResourceHandle = void*;
#else
using GraphicsResourceHandle = uint32;
#endif

class GraphicsObject
{
public:
	explicit GraphicsObject(Graphics* pGraphics);
    void SetName(const std::string& name);

	GraphicsResourceHandle GetResource() const { return m_pResource; }

protected:
	Graphics* m_pGraphics;
	GraphicsResourceHandle m_pResource{};
};