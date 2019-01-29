#pragma once

class Graphics;

class BlendState
{
public:
	BlendState();
	~BlendState();

	DELETE_COPY(BlendState)

	void SetBlendMode(const BlendMode& blendMode, bool alphaToCoverage);
	void SetColorWrite(ColorWrite colorWriteMask = ColorWrite::ALL);

	void* GetOrCreate(Graphics* pGraphics);
	bool IsDirty() const { return m_IsDirty; }

private:
	bool m_IsDirty = true;

	BlendMode m_BlendMode = BlendMode::REPLACE;
	bool m_AlphaToCoverage = false;
	ColorWrite m_ColorWriteMask = ColorWrite::ALL;

	std::map<unsigned int, void*> m_BlendStates;
};