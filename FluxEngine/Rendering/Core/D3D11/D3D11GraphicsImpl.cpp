#include "FluxEngine.h"
#include "D3D11GraphicsImpl.h"

unsigned int GraphicsImpl::GetMultisampleQuality(const DXGI_FORMAT format, const unsigned int sampleCount) const
{
	if (sampleCount < 2)
	{
		return 0;
	}

	UINT numLevels = 0;
	HRESULT hr = m_pDevice->CheckMultisampleQualityLevels(format, sampleCount, &numLevels);
	if (hr != S_OK || !numLevels)
	{
		return 0;
	}
	else
	{
		return numLevels - 1;
	}
}

bool GraphicsImpl::CheckMultisampleQuality(const DXGI_FORMAT format, const unsigned int sampleCount) const
{
	if (sampleCount < 2)
	{
		return true; // Not multisampled, should use quality 0
	}

	UINT numLevels = 0;
	HRESULT hr = m_pDevice->CheckMultisampleQualityLevels(format, sampleCount, &numLevels);
	if (hr != S_OK)
	{
		return false; // Errored or sample count not supported
	}
	else
	{
		return numLevels > 0; // D3D10.0 and below: use the best quality
	}
}