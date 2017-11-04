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

	void SetScissorEnabled(const bool enabled);
	void SetMultisampleEnabled(const bool enabled);
	void SetFillMode(const FillMode fillMode);
	void SetCullMode(const CullMode cullMode);
	void SetLineAntialias(const bool lineAntiAlias);

private:
	bool m_IsDirty = true;
	bool m_LineAntiAlias = false;
	bool m_ScissorEnabled = false;
	bool m_MultisampleEnabled = true;

	FillMode m_FillMode = FillMode::SOLID;
	CullMode m_CullMode = CullMode::BACK;

	map<unsigned int, void*> m_RasterizerStates;
};