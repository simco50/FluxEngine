#pragma once
#include "GraphicsDefines.h"

class Graphics;

class RasterizerState
{
public:
	RasterizerState();
	~RasterizerState();

	DELETE_COPY(RasterizerState)

	void* GetOrCreate(Graphics* pGraphics);
	bool IsDirty() const { return m_IsDirty; }

	void SetScissorEnabled(bool enabled);
	void SetMultisampleEnabled(bool enabled);
	void SetFillMode(FillMode fillMode);
	void SetCullMode(CullMode cullMode);
	void SetLineAntialias(bool lineAntiAlias);

private:
	bool m_IsDirty = true;
	bool m_LineAntiAlias = false;
	bool m_ScissorEnabled = false;
	bool m_MultisampleEnabled = true;

	FillMode m_FillMode = FillMode::SOLID;
	CullMode m_CullMode = CullMode::BACK;

	std::map<unsigned int, void*> m_RasterizerStates;
};