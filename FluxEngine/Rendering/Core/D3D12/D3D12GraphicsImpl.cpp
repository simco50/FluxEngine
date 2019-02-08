#include "FluxEngine.h"
#include "D3D12GraphicsImpl.h"

bool GraphicsImpl::GetPrimitiveType(const PrimitiveType primitiveType, const unsigned int elementCount, D3D12_PRIMITIVE_TOPOLOGY& type, unsigned int& primitiveCount)
{
	switch (primitiveType)
	{
	case PrimitiveType::TRIANGLELIST:
		type = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		primitiveCount = elementCount / 3;
		return true;
	case PrimitiveType::POINTLIST:
		type = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		primitiveCount = elementCount / 2;
		return true;
	case PrimitiveType::TRIANGLESTRIP:
		type = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		primitiveCount = elementCount - 2;
		return true;
	case PrimitiveType::LINELIST:
		type = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		primitiveCount = elementCount / 2;
		return true;
	case PrimitiveType::PATCH_CP_3:
		type = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
		primitiveCount = elementCount;
		return true;
	default:
		FLUX_LOG(Error, "[Graphics::SetPrimitiveType()] > Invalid primitive type");
		return false;
	}
}

unsigned int GraphicsImpl::GetMultisampleQuality(const DXGI_FORMAT format, const unsigned int sampleCount) const
{
	AUTOPROFILE(Graphics_Impl_GetMultisampleQuality);

	if (sampleCount < 2)
	{
		return 0;
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS data = {};
	data.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	data.Format = format;
	data.NumQualityLevels = 0;
	data.SampleCount = sampleCount;
	HRESULT hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &data, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	if (hr != S_OK || !data.NumQualityLevels)
	{
		return 0;
	}
	else
	{
		return data.NumQualityLevels - 1;
	}
}

bool GraphicsImpl::CheckMultisampleQuality(const DXGI_FORMAT format, const unsigned int sampleCount) const
{
	AUTOPROFILE(Graphics_Impl_CheckMultisampleQuality);

	if (sampleCount < 2)
	{
		return true; // Not multisampled, should use quality 0
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS data = {};
	data.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	data.Format = format;
	data.NumQualityLevels = 0;
	data.SampleCount = sampleCount;
	HRESULT hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &data, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	if (hr != S_OK)
	{
		return false; // Errored or sample count not supported
	}
	else
	{
		return data.NumQualityLevels > 0; // D3D10.0 and below: use the best quality
	}
}

ID3D12GraphicsCommandList* GraphicsImpl::GetTemporaryCommandList()
{
	return nullptr;
}
