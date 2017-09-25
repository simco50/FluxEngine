#pragma once

class View;

class RenderViewport
{
public:
	RenderViewport(Camera* pCamera, const IntRect& viewport);
	~RenderViewport();

	DELETE_COPY(RenderViewport)

	Camera* GetCamera() const { return m_pCamera; }
	const IntRect& GetRect() const { return m_Viewport; }
	void SetRect(const IntRect& rect) { m_Viewport = rect; }

	View* GetView() const { return m_pView.get(); }
	View* AllocateView();

private:

	Camera* m_pCamera;
	IntRect m_Viewport;
	unique_ptr<View> m_pView;
};