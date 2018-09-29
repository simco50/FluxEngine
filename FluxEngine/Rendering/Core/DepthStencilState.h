#pragma once
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

	void SetDepthEnabled(bool enabled);
	void SetDepthWrite(bool enabled);
	void SetDepthTest(const CompareMode& comparison);
	void SetStencilTest(bool stencilEnabled, CompareMode mode, StencilOperation pass, StencilOperation fail, StencilOperation zFail, unsigned int stencilRef, unsigned char compareMask, unsigned char writeMask);
private:
	bool m_IsDirty = true;

	bool m_DepthEnabled = true;
	bool m_DepthWrite = true;
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