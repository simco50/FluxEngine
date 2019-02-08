#include "FluxEngine.h"
#include "D3D11InputLayout.h"
#include "D3D11GraphicsImpl.h"
#include "../Graphics.h"
#include "../ShaderVariation.h"
#include "../VertexBuffer.h"
#include "../D3DCommon/D3DDefines.h"

InputLayout::InputLayout(Graphics* pGraphics)
	: GraphicsObject(pGraphics)
{
}

InputLayout::~InputLayout()
{
	SafeRelease(m_pResource);
}

void InputLayout::Create(VertexBuffer** vertexBuffers, unsigned int bufferCount, ShaderVariation* pVariation)
{
	AUTOPROFILE_DESC(InputLayout_Create, pVariation->GetName());

	SafeRelease(m_pResource);

	std::vector<D3D11_INPUT_ELEMENT_DESC> elementDesc;

	for (unsigned int i = 0; i < bufferCount; ++i)
	{
		if (vertexBuffers[i] == nullptr)
			continue;

		for (const VertexElement& e : vertexBuffers[i]->GetElements())
		{
			D3D11_INPUT_ELEMENT_DESC desc;
			desc.SemanticName = D3DCommon::GetSemanticOfType(e.Semantic);
			desc.Format = D3DCommon::GetFormatOfType(e.Type);
			desc.AlignedByteOffset = e.Offset;
			desc.InputSlotClass = e.PerInstance ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
			desc.InputSlot = i;
			desc.InstanceDataStepRate = e.PerInstance ? 1 : 0;
			desc.SemanticIndex = e.Index;

			elementDesc.push_back(desc);
		}
	}
	const std::vector<char>& byteCode = pVariation->GetByteCode();
	HR(m_pGraphics->GetImpl()->GetDevice()->CreateInputLayout(elementDesc.data(), (UINT)elementDesc.size(), byteCode.data(), (UINT)byteCode.size(), (ID3D11InputLayout**)&m_pResource));
}

void InputLayout::Create(std::vector<VertexBuffer*> vertexBuffers, ShaderVariation* pVariation)
{
	Create(vertexBuffers.data(), (unsigned int)vertexBuffers.size(), pVariation);
}