#include "stdafx.h"
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

void InputLayout::Create(const vector<VertexBuffer*>& vertexBuffers, ShaderVariation* pVariation)
{
	vector<D3D11_INPUT_ELEMENT_DESC> elementDesc;

	for (unsigned int i = 0; i < vertexBuffers.size(); ++i)
	{
		for (const VertexElement& e : vertexBuffers[i]->GetElements())
		{
			D3D11_INPUT_ELEMENT_DESC desc;
			desc.SemanticName = VertexElement::GetSemanticOfType(e.Semantic);
			desc.Format = VertexElement::GetFormatOfType(e.Type);
			desc.AlignedByteOffset = e.Offset;
			desc.InputSlotClass = e.PerInstance ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
			desc.InputSlot = i;
			desc.InstanceDataStepRate = e.PerInstance ? 1 : 0;
			desc.SemanticIndex = e.Index;

			elementDesc.push_back(desc);
		}
	}
	const vector<unsigned char>& byteCode = pVariation->GetByteCode();
	HR(m_pGraphics->GetDevice()->CreateInputLayout(elementDesc.data(), (UINT)elementDesc.size(), byteCode.data(), (unsigned int)byteCode.size(), (ID3D11InputLayout**)&m_pInputLayout))
}
