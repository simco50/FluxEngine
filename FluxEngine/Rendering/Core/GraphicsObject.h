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
	explicit GraphicsObject(Graphics* pGraphics);
    void SetName(const std::string& name);

#if defined(GRAPHICS_D3D11) || defined(GRAPHICS_D3D12)
	void* GetResource() const { return m_pResource; }
#else
	unsigned int GetResource() const { return m_Handle; }
#endif

protected:
	Graphics* m_pGraphics;

	union
	{
		void* m_pResource = nullptr;
		unsigned int m_Handle;
	};
};