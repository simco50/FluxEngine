#pragma once

inline std::string D3DBlobToString(ID3DBlob* pBlob)
{
	std::string data;
	data.resize(pBlob->GetBufferSize());
	memcpy(&data[0], pBlob->GetBufferPointer(), pBlob->GetBufferSize());
	return data;
}

inline void D3DBlobToVector(ID3DBlob* pBlob, vector<char>& buffer)
{
	buffer.resize(pBlob->GetBufferSize());
	memcpy(buffer.data(), pBlob->GetBufferPointer(), buffer.size());
}

inline D3D11_COMPARISON_FUNC D3D11ComparisonFunction(CompareMode mode)
{
	switch (mode)
	{
	case CompareMode::ALWAYS:			return D3D11_COMPARISON_ALWAYS;
	case CompareMode::EQUAL:			return D3D11_COMPARISON_EQUAL;
	case CompareMode::NOTEQUAL:			return D3D11_COMPARISON_NOT_EQUAL;
	case CompareMode::LESS:				return D3D11_COMPARISON_LESS;
	case CompareMode::LESSEQUAL:		return D3D11_COMPARISON_LESS_EQUAL;
	case CompareMode::GREATER:			return D3D11_COMPARISON_GREATER;
	case CompareMode::GREATEREQUAL:		return D3D11_COMPARISON_GREATER_EQUAL;
	}
	return D3D11_COMPARISON_LESS;
}

inline D3D11_FILL_MODE D3D11FillMode(FillMode mode)
{
	switch (mode)
	{
	case FillMode::SOLID: return D3D11_FILL_SOLID;
	case FillMode::WIREFRAME: return D3D11_FILL_WIREFRAME;
	}
	return D3D11_FILL_SOLID;
}

inline D3D11_CULL_MODE D3D11CullMode(CullMode mode)
{
	switch (mode)
	{
	case CullMode::BACK: return D3D11_CULL_BACK;
	case CullMode::NONE: return D3D11_CULL_NONE;
	case CullMode::FRONT: return D3D11_CULL_FRONT;
	}
	return D3D11_CULL_BACK;
}

inline D3D11_STENCIL_OP D3D11StencilOperation(StencilOperation operation)
{
	switch (operation)
	{
	case StencilOperation::KEEP:		return D3D11_STENCIL_OP_KEEP;
	case StencilOperation::ZERO:		return D3D11_STENCIL_OP_ZERO;
	case StencilOperation::REF:			return D3D11_STENCIL_OP_REPLACE;
	case StencilOperation::INCR:		return D3D11_STENCIL_OP_INCR;
	case StencilOperation::DECR:		return D3D11_STENCIL_OP_DECR;
	}
	return D3D11_STENCIL_OP_REPLACE;
}

inline D3D11_FILTER D3D11Filter(TextureFilter filter)
{
	switch (filter)
	{
	case TextureFilter::MIN_MAG_MIP_POINT:								return D3D11_FILTER_MIN_MAG_MIP_POINT;
	case TextureFilter::MIN_MAG_POINT_MIP_LINEAR:						return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	case TextureFilter::MIN_POINT_MAG_LINEAR_MIP_POINT:					return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
	case TextureFilter::MIN_POINT_MAG_MIP_LINEAR:						return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	case TextureFilter::MIN_LINEAR_MAG_MIP_POINT:						return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	case TextureFilter::MIN_LINEAR_MAG_POINT_MIP_LINEAR:				return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	case TextureFilter::MIN_MAG_LINEAR_MIP_POINT:						return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	case TextureFilter::MIN_MAG_MIP_LINEAR:								return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	case TextureFilter::ANISOTROPIC:									return D3D11_FILTER_ANISOTROPIC;
	case TextureFilter::COMPARISON_MIN_MAG_MIP_POINT:					return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	case TextureFilter::COMPARISON_MIN_MAG_POINT_MIP_LINEAR:			return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
	case TextureFilter::COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:		return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
	case TextureFilter::COMPARISON_MIN_POINT_MAG_MIP_LINEAR:			return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
	case TextureFilter::COMPARISON_MIN_LINEAR_MAG_MIP_POINT:			return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
	case TextureFilter::COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:		return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	case TextureFilter::COMPARISON_MIN_MAG_LINEAR_MIP_POINT:			return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	case TextureFilter::COMPARISON_MIN_MAG_MIP_LINEAR:					return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	case TextureFilter::COMPARISON_ANISOTROPIC:							return D3D11_FILTER_COMPARISON_ANISOTROPIC;
	case TextureFilter::MINIMUM_MIN_MAG_MIP_POINT:						return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
	case TextureFilter::MINIMUM_MIN_MAG_POINT_MIP_LINEAR:				return D3D11_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
	case TextureFilter::MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:			return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
	case TextureFilter::MINIMUM_MIN_POINT_MAG_MIP_LINEAR:				return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
	case TextureFilter::MINIMUM_MIN_LINEAR_MAG_MIP_POINT:				return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
	case TextureFilter::MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:		return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	case TextureFilter::MINIMUM_MIN_MAG_LINEAR_MIP_POINT:				return D3D11_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
	case TextureFilter::MINIMUM_MIN_MAG_MIP_LINEAR:						return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
	case TextureFilter::MINIMUM_ANISOTROPIC:							return D3D11_FILTER_MINIMUM_ANISOTROPIC;
	case TextureFilter::MAXIMUM_MIN_MAG_MIP_POINT:						return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
	case TextureFilter::MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:				return D3D11_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
	case TextureFilter::MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:			return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
	case TextureFilter::MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:				return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
	case TextureFilter::MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:				return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
	case TextureFilter::MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:		return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	case TextureFilter::MAXIMUM_MIN_MAG_LINEAR_MIP_POINT:				return D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
	case TextureFilter::MAXIMUM_MIN_MAG_MIP_LINEAR:						return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
	case TextureFilter::MAXIMUM_ANISOTROPIC:							return D3D11_FILTER_MAXIMUM_ANISOTROPIC;
	}
	return D3D11_FILTER_ANISOTROPIC;
}

inline D3D11_RENDER_TARGET_BLEND_DESC D3D11RenderTargetBlendDesc(BlendMode mode, unsigned char colorWriteMask)
{
	D3D11_RENDER_TARGET_BLEND_DESC desc = {};
	desc.RenderTargetWriteMask = colorWriteMask;
	desc.BlendEnable = mode == BlendMode::REPLACE ? false : true;

	switch (mode)
	{
	case BlendMode::REPLACE:
		desc.SrcBlend = D3D11_BLEND_ONE;
		desc.DestBlend = D3D11_BLEND_ZERO;
		desc.BlendOp = D3D11_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::ALPHA:
		desc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.BlendOp = D3D11_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		desc.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::ADD:
		desc.SrcBlend = D3D11_BLEND_ONE;
		desc.DestBlend = D3D11_BLEND_ONE;
		desc.BlendOp = D3D11_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.DestBlendAlpha = D3D11_BLEND_ONE;
		desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::MULTIPLY:
		desc.SrcBlend = D3D11_BLEND_DEST_COLOR;
		desc.DestBlend = D3D11_BLEND_ZERO;
		desc.BlendOp = D3D11_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D11_BLEND_DEST_COLOR;
		desc.DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::ADDALPHA:
		desc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.DestBlend = D3D11_BLEND_ONE;
		desc.BlendOp = D3D11_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		desc.DestBlendAlpha = D3D11_BLEND_ONE;
		desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::PREMULALPHA:
		desc.SrcBlend = D3D11_BLEND_ONE;
		desc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.BlendOp = D3D11_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::INVDESTALPHA:
		desc.SrcBlend = D3D11_BLEND_INV_DEST_ALPHA;
		desc.DestBlend = D3D11_BLEND_DEST_ALPHA;
		desc.BlendOp = D3D11_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
		desc.DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
		desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::SUBTRACT:
		desc.SrcBlend = D3D11_BLEND_ONE;
		desc.DestBlend = D3D11_BLEND_ONE;
		desc.BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		desc.SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.DestBlendAlpha = D3D11_BLEND_ONE;
		desc.BlendOpAlpha = D3D11_BLEND_OP_REV_SUBTRACT;
		break;
	case BlendMode::SUBTRACTALPHA:
		desc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.DestBlend = D3D11_BLEND_ONE;
		desc.BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		desc.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		desc.DestBlendAlpha = D3D11_BLEND_ONE;
		desc.BlendOpAlpha = D3D11_BLEND_OP_REV_SUBTRACT;
		break;
	}
	return desc;
}