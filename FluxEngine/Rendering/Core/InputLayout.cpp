#include "FluxEngine.h"
#include "InputLayout.h"

InputLayout::InputLayout(Graphics* pGraphics)
	: m_pGraphics(pGraphics)
{
}

InputLayout::~InputLayout()
{
	SafeRelease(m_pInputLayout);
}