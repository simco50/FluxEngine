#include "FluxEngine.h"
#include "D3D12GraphicsImpl.h"

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
