#include "GraphicsDefines.h"

class BlendState
{
public:
	BlendState();

	void SetBlendMode(const BlendMode& blendMode, const bool alphaToCoverage);
	void SetColorWrite(const ColorWrite colorWriteMask = ColorWrite::ALL);

	ID3D11BlendState* Create(ID3D11Device* pDevice);
	bool IsDirty() const { return m_IsDirty; }

private:
	
	ComPtr<ID3D11BlendState> m_pBlendState;

	BlendMode m_BlendMode = BlendMode::REPLACE;
	bool m_AlphaToCoverage = false;
	ColorWrite m_ColorWriteMask = ColorWrite::ALL;

	bool m_IsDirty = true;
};