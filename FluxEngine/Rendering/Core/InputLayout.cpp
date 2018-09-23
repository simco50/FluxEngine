#include "FluxEngine.h"
#include "InputLayout.h"
#include "VertexBuffer.h"
#include "ShaderVariation.h"
#include "Graphics.h"

InputLayout::InputLayout(Graphics* pGraphics) :
	m_pGraphics(pGraphics)
{
}


InputLayout::~InputLayout()
{
	SafeRelease(m_pInputLayout);
}