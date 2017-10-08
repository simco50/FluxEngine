#include "GraphicsDefines.h"

class RasterizerState
{
public:
	RasterizerState();

	ID3D11RasterizerState* Create(ID3D11Device* pDevice);
	bool IsDirty() const { return m_IsDirty; }

	void SetScissorEnabled(const bool enabled);
	void SetMultisampleEnabled(const bool enabled);
	void SetFillMode(const FillMode fillMode);
	void SetCullMode(const CullMode cullMode);

private:
	ComPtr<ID3D11RasterizerState> m_pRasterizerState;
	bool m_IsDirty = true;

	bool m_ScissorEnabled = false;
	bool m_MultisampleEnabled = true;

	FillMode m_FillMode = FillMode::SOLID;
	CullMode m_CullMode = CullMode::BACK;
};