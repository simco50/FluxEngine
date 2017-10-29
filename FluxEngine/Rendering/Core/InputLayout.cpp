#include "stdafx.h"
#include "InputLayout.h"
#include "VertexBuffer.h"
#include "ShaderVariation.h"
#include "Graphics.h"

#ifdef D3D11
#include "D3D11/D3D11InputLayout.hpp"
#endif

InputLayout::InputLayout(Graphics* pGraphics) : 
	m_pGraphics(pGraphics)
{
}


InputLayout::~InputLayout()
{
	SafeRelease(m_pInputLayout);
}