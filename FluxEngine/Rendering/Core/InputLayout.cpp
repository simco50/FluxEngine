#include "FluxEngine.h"
#include "InputLayout.h"

InputLayout::InputLayout(Graphics* pGraphics)
	: GraphicsObject(pGraphics)
{
}

InputLayout::~InputLayout()
{
	SafeRelease(m_pResource);
}