#include "GraphicsDefines.h"

class BlendState
{
public:
	BlendState();

	DELETE_COPY(BlendState)

	void SetBlendMode(const BlendMode& blendMode, const bool alphaToCoverage);
	void SetColorWrite(const ColorWrite colorWriteMask = ColorWrite::ALL);

	ID3D11BlendState* GetOrCreate(ID3D11Device* pDevice);
	bool IsDirty() const { return m_IsDirty; }

private:
	bool m_IsDirty = true;

	BlendMode m_BlendMode = BlendMode::REPLACE;
	bool m_AlphaToCoverage = false;
	ColorWrite m_ColorWriteMask = ColorWrite::ALL;

	map<unsigned int, ComPtr<ID3D11BlendState>> m_BlendStates;
};