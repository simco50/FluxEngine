#pragma once

class Graphics;

#ifdef _DEBUG
#define GRAPHICS_SET_NAME(name) SetName(name);
#else
#define GRAPHICS_SET_NAME(name)
#endif

class GraphicsObject
{
public:
	GraphicsObject(Graphics* pGraphics);
    void SetName(const std::string& name);

#ifdef GRAPHICS_D3D11
	ID3D11Resource* GetResource() const { return m_pResource; }
#endif

protected:
	Graphics* m_pGraphics;
#ifdef GRAPHICS_D3D11
    ID3D11Resource* m_pResource = nullptr;
#endif
};