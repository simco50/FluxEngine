#pragma once

class RenderViewport;

class Renderer
{
public:
	Renderer();
	~Renderer();

	DELETE_COPY(Renderer)

	void SetViewport(const unsigned int index, RenderViewport* pViewport);
	void Render();

private:

	vector<RenderViewport*> m_Viewports;
};