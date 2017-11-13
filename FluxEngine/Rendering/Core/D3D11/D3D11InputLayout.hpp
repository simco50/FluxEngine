#include "D3D11GraphicsImpl.h"
void InputLayout::Create(VertexBuffer** vertexBuffers, const unsigned int bufferCount, ShaderVariation* pVariation)
{
	AUTOPROFILE_DESC(InputLayout_Create, pVariation->GetName());

	SafeRelease(m_pInputLayout);

	vector<D3D11_INPUT_ELEMENT_DESC> elementDesc;

	for (unsigned int i = 0; i < bufferCount; ++i)
	{
		if (vertexBuffers[i] == nullptr)
			continue;

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
	const vector<char>& byteCode = pVariation->GetByteCode();
	HR(m_pGraphics->GetImpl()->GetDevice()->CreateInputLayout(elementDesc.data(), (UINT)elementDesc.size(), byteCode.data(), (unsigned int)byteCode.size(), (ID3D11InputLayout**)&m_pInputLayout))
}

void InputLayout::Create(vector<VertexBuffer*> vertexBuffers, ShaderVariation* pVariation)
{
	Create(vertexBuffers.data(), (unsigned int)vertexBuffers.size(), pVariation);
}

