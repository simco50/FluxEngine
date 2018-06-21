#include "FluxEngine.h"
#include "Rendering/Core/InputLayout.h"
#include "Rendering/Core/ShaderProgram.h"
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
	case PrimitiveType::LINELIST:
		type = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		primitiveCount = elementCount / 2;
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
		return 0;

	UINT numLevels = 0;
	HRESULT hr = m_pDevice->CheckMultisampleQualityLevels(format, sampleCount, &numLevels);
	if (hr != S_OK || !numLevels)
		return 0;
	else
		return numLevels - 1;
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