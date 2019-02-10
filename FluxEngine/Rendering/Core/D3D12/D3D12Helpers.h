#pragma once

inline constexpr D3D12_COMPARISON_FUNC D3D12ComparisonFunction(CompareMode mode)
{
	switch (mode)
	{
	case CompareMode::ALWAYS:			return D3D12_COMPARISON_FUNC_ALWAYS;
	case CompareMode::EQUAL:			return D3D12_COMPARISON_FUNC_EQUAL;
	case CompareMode::NOTEQUAL:			return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case CompareMode::LESS:				return D3D12_COMPARISON_FUNC_LESS;
	case CompareMode::LESSEQUAL:		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case CompareMode::GREATER:			return D3D12_COMPARISON_FUNC_GREATER;
	case CompareMode::GREATEREQUAL:		return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case CompareMode::UNDEFINED:
	default:							return D3D12_COMPARISON_FUNC_LESS;
	}
}

inline constexpr D3D12_FILL_MODE D3D12FillMode(FillMode mode)
{
	switch (mode)
	{
	case FillMode::SOLID: return D3D12_FILL_MODE_SOLID;
	case FillMode::WIREFRAME: return D3D12_FILL_MODE_WIREFRAME;
	default: return D3D12_FILL_MODE_SOLID;
	}
}

inline constexpr D3D12_CULL_MODE D3D12CullMode(CullMode mode)
{
	switch (mode)
	{
	case CullMode::BACK: return D3D12_CULL_MODE_BACK;
	case CullMode::NONE: return D3D12_CULL_MODE_NONE;
	case CullMode::FRONT: return D3D12_CULL_MODE_FRONT;
	default: return D3D12_CULL_MODE_BACK;
	}
}

inline constexpr D3D12_STENCIL_OP D3D12StencilOperation(StencilOperation operation)
{
	switch (operation)
	{
	case StencilOperation::KEEP:		return D3D12_STENCIL_OP_KEEP;
	case StencilOperation::ZERO:		return D3D12_STENCIL_OP_ZERO;
	case StencilOperation::REF:			return D3D12_STENCIL_OP_REPLACE;
	case StencilOperation::INCR:		return D3D12_STENCIL_OP_INCR;
	case StencilOperation::DECR:		return D3D12_STENCIL_OP_DECR;
	}
	return D3D12_STENCIL_OP_REPLACE;
}

inline constexpr D3D12_RENDER_TARGET_BLEND_DESC D3D12RenderTargetBlendDesc(BlendMode mode, unsigned char colorWriteMask)
{
	D3D12_RENDER_TARGET_BLEND_DESC desc = {};
	desc.RenderTargetWriteMask = colorWriteMask;
	desc.BlendEnable = mode == BlendMode::REPLACE ? false : true;

	switch (mode)
	{
	case BlendMode::REPLACE:
		desc.SrcBlend = D3D12_BLEND_ONE;
		desc.DestBlend = D3D12_BLEND_ZERO;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D12_BLEND_ONE;
		desc.DestBlendAlpha = D3D12_BLEND_ZERO;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;
	case BlendMode::ALPHA:
		desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		desc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
		desc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;
	case BlendMode::ADD:
		desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		desc.DestBlend = D3D12_BLEND_ONE;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D12_BLEND_ONE;
		desc.DestBlendAlpha = D3D12_BLEND_ONE;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;
	case BlendMode::MULTIPLY:
		desc.SrcBlend = D3D12_BLEND_DEST_COLOR;
		desc.DestBlend = D3D12_BLEND_ZERO;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D12_BLEND_DEST_COLOR;
		desc.DestBlendAlpha = D3D12_BLEND_ZERO;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;
	case BlendMode::ADDALPHA:
		desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		desc.DestBlend = D3D12_BLEND_ONE;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
		desc.DestBlendAlpha = D3D12_BLEND_ONE;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;
	case BlendMode::PREMULALPHA:
		desc.SrcBlend = D3D12_BLEND_ONE;
		desc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D12_BLEND_ONE;
		desc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;
	case BlendMode::INVDESTALPHA:
		desc.SrcBlend = D3D12_BLEND_INV_DEST_ALPHA;
		desc.DestBlend = D3D12_BLEND_DEST_ALPHA;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D12_BLEND_INV_DEST_ALPHA;
		desc.DestBlendAlpha = D3D12_BLEND_DEST_ALPHA;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;
	case BlendMode::SUBTRACT:
		desc.SrcBlend = D3D12_BLEND_ONE;
		desc.DestBlend = D3D12_BLEND_ONE;
		desc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		desc.SrcBlendAlpha = D3D12_BLEND_ONE;
		desc.DestBlendAlpha = D3D12_BLEND_ONE;
		desc.BlendOpAlpha = D3D12_BLEND_OP_REV_SUBTRACT;
		break;
	case BlendMode::SUBTRACTALPHA:
		desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		desc.DestBlend = D3D12_BLEND_ONE;
		desc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		desc.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
		desc.DestBlendAlpha = D3D12_BLEND_ONE;
		desc.BlendOpAlpha = D3D12_BLEND_OP_REV_SUBTRACT;
		break;
	case BlendMode::UNDEFINED:
	default:
		return desc;
	}
	return desc;
}