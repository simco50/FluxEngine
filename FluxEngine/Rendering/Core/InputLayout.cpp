#include "stdafx.h"
#include "InputLayout.h"
#include "VertexBuffer.h"
#include "ShaderVariation.h"


InputLayout::InputLayout(ID3D11Device* pDevice) : 
	m_pDevice(pDevice)
{
}


InputLayout::~InputLayout()
{
	
}

void InputLayout::Create(const vector<VertexBuffer*>& vertexBuffers, ShaderVariation* pVariation)
{
	vector<D3D11_INPUT_ELEMENT_DESC> elementDesc;

	for (unsigned int i = 0; i < vertexBuffers.size(); ++i)
	{
		for (const VertexElement& e : vertexBuffers[i]->GetElements())
		{
			D3D11_INPUT_ELEMENT_DESC desc;
			switch (e.Semantic)
			{
			case VertexElementSemantic::POSITION:
				desc.SemanticName = "POSITION";
				desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				break;
			case VertexElementSemantic::NORMAL:
				desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				desc.SemanticName = "NORMAL";
				break;
			case VertexElementSemantic::BINORMAL:
				desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				desc.SemanticName = "BINORMAL";
				break;
			case VertexElementSemantic::TANGENT:
				desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				desc.SemanticName = "TANGENT";
				break;
			case VertexElementSemantic::TEXCOORD:
				desc.Format = DXGI_FORMAT_R32G32_FLOAT;
				desc.SemanticName = "TEXCOORD";
				break;
			case VertexElementSemantic::COLOR:
				desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				desc.SemanticName = "COLOR";
				break;
			case VertexElementSemantic::BLENDWEIGHTS:
				desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				desc.SemanticName = "BLENDWEIGHT";
				break;
			case VertexElementSemantic::BLENDINDICES:
				desc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				desc.SemanticName = "BLENDINDEX";
				break;
			case VertexElementSemantic::OBJECTINDEX:
				desc.Format = DXGI_FORMAT_R32_UINT;
				desc.SemanticName = "OBJECTINDEX";
				break;
			case VertexElementSemantic::MAX_VERTEX_ELEMENT_SEMANTICS:
				break;
			default:
				break;

			}

			desc.AlignedByteOffset = e.Offset;
			desc.InputSlotClass = e.PerInstance ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
			desc.InputSlot = i;
			desc.InstanceDataStepRate = e.PerInstance ? 1 : 0;
			desc.SemanticIndex = e.Index;

			elementDesc.push_back(desc);
		}
	}
	const vector<unsigned char>& byteCode = pVariation->GetByteCode();
	HR(m_pDevice->CreateInputLayout(elementDesc.data(), elementDesc.size(), byteCode.data(), byteCode.size(), m_pInputLayout.GetAddressOf()))
}
