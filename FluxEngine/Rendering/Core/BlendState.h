#pragma once
#include "GraphicsDefines.h"

class Graphics;

class BlendState
{
public:
	BlendState();
	~BlendState();

	DELETE_COPY(BlendState)

	void SetBlendMode(const BlendMode& blendMode, const bool alphaToCoverage);
	void SetColorWrite(const ColorWrite colorWriteMask = ColorWrite::ALL);

	void* GetOrCreate(Graphics* pGraphics);
	bool IsDirty() const { return m_IsDirty; }

private:
	bool m_IsDirty = true;

	BlendMode m_BlendMode = BlendMode::REPLACE;
	bool m_AlphaToCoverage = false;
	ColorWrite m_ColorWriteMask = ColorWrite::ALL;

	map<unsigned int, void*> m_BlendStates;
};