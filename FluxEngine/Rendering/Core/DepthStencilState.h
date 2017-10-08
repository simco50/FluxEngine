#include "GraphicsDefines.h"

class Graphics;

class DepthStencilState
{
public:
	DepthStencilState();

	DELETE_COPY(DepthStencilState)

	ID3D11DepthStencilState* GetOrCreate(ID3D11Device* pDevice);
	bool IsDirty() const { return m_IsDirty; }
	unsigned int GetStencilRef() const { return m_StencilRef; }

	void SetDepthEnabled(const bool enabled);
	void SetDepthTest(const CompareMode& comparison);
	void SetStencilTest(bool stencilEnabled, const CompareMode mode, const StencilOperation pass, const StencilOperation fail, const StencilOperation zFail, const unsigned int stencilRef, const unsigned char compareMask, const unsigned char writeMask);
private:
	bool m_IsDirty = true;

	bool m_DepthEnabled = true;
	CompareMode m_DepthCompareMode = CompareMode::LESS;
	bool m_StencilTestEnabled = false;
	CompareMode m_StencilTestMode = CompareMode::ALWAYS;
	StencilOperation m_StencilTestPassOperation = StencilOperation::KEEP;
	StencilOperation m_StencilTestFailOperation = StencilOperation::KEEP;
	StencilOperation m_StencilTestZFailOperation = StencilOperation::KEEP;
	unsigned int m_StencilRef = 0;
	unsigned char m_StencilCompareMask;
	unsigned char m_StencilWriteMask;

	map<unsigned int, ComPtr<ID3D11DepthStencilState>> m_DepthStencilStates;
};