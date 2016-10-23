#pragma once

struct ILElement
{
	UINT Size = 0;
	DXGI_FORMAT Format;
};

struct ILDesc
{
	enum ILSemantic : UINT
	{
		POSITION = 1 << 0,
		NORMAL = 1 << 1,
		COLOR = 1 << 2,
		TEXCOORD = 1 << 3,
		TANGENT = 1 << 4,
	};
	UINT ElementSemantics = 0;
	UINT VertexStride = 0;
	vector<D3D11_INPUT_ELEMENT_DESC> InputLayoutDesc;
	//vector<ILElement> InputLayoutElements;
};