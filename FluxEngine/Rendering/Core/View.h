#pragma once
class RenderViewport;

class View
{
public:
	View();
	~View();

	DELETE_COPY(View)

	void Render();
	void Define(RenderViewport* pViewport);

private:
	RenderViewport* m_pViewport = nullptr;
};