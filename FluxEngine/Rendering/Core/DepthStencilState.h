#include "GraphicsDefines.h"

class Graphics;

class DepthStencilState
{
public:
	DepthStencilState();
	~DepthStencilState();

	DELETE_COPY(DepthStencilState)

	void* GetOrCreate(Graphics* pGraphics);
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
	unsigned char m_StencilCompareMask = 0;
	unsigned char m_StencilWriteMask = 0;

	std::map<unsigned int, void*> m_DepthStencilStates;
};