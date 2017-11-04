#pragma once

struct InputLayoutElement
{
	UINT Size = 0;
	DXGI_FORMAT Format;
};

struct InputLayoutDesc
{
	enum LayoutSemantic : UINT
	{
		POSITION = 1 << 0,
		NORMAL = 1 << 1,
		COLOR = 1 << 2,
		TEXCOORD = 1 << 3,
		TANGENT = 1 << 4,
	};
	UINT ElementSemantics = 0;
	UINT VertexStride = 0;
	std::vector<D3D11_INPUT_ELEMENT_DESC> LayoutDesc;
};