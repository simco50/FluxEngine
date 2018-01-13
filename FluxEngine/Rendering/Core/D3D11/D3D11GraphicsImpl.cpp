#include "FluxEngine.h"
#include "D3D11GraphicsImpl.h"

bool GraphicsImpl::GetPrimitiveType(const PrimitiveType primitiveType, const unsigned int elementCount, D3D11_PRIMITIVE_TOPOLOGY& type, unsigned int& primitiveCount)
{
	switch (primitiveType)
	{
	case PrimitiveType::TRIANGLELIST:
		type = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		primitiveCount = elementCount / 3;
		return true;
	case PrimitiveType::POINTLIST:
		type = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		primitiveCount = elementCount / 2;
		return true;
	case PrimitiveType::TRIANGLESTRIP:
		type = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		primitiveCount = elementCount - 2;
		return true;
	default:
		FLUX_LOG(ERROR, "[Graphics::SetPrimitiveType()] > Invalid primitive type");
		return false;
	}
}

unsigned int GraphicsImpl::GetMultisampleQuality(const DXGI_FORMAT format, const unsigned int sampleCount) const
{
	AUTOPROFILE(Graphics_Impl_GetMultisampleQuality);

	if (sampleCount < 2)
		return 0; // Not multisampled, should use quality 0

	if (m_pDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_10_1)
		return 0xffffffff; // D3D10.1+ standard level

	UINT numLevels = 0;
	HRESULT hr = m_pDevice->CheckMultisampleQualityLevels(format, sampleCount, &numLevels);
	if (hr != S_OK || !numLevels)
		return 0; // Errored or sample count not supported
	else
		return numLevels - 1; // D3D10.0 and below: use the best quality
}

bool GraphicsImpl::CheckMultisampleQuality(const DXGI_FORMAT format, const unsigned int sampleCount) const
{
	AUTOPROFILE(Graphics_Impl_CheckMultisampleQuality);

	if (sampleCount < 2)
		return true; // Not multisampled, should use quality 0

	UINT numLevels = 0;
	HRESULT hr = m_pDevice->CheckMultisampleQualityLevels(format, sampleCount, &numLevels);
	if (hr != S_OK)
		return false; // Errored or sample count not supported
	else
		return numLevels > 0; // D3D10.0 and below: use the best quality
}