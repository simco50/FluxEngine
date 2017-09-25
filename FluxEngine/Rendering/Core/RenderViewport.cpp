#include "stdafx.h"
#include "RenderViewport.h"
#include "View.h"


RenderViewport::RenderViewport(Camera* pCamera, const IntRect& viewport) :
	m_pCamera(pCamera), m_Viewport(viewport)
{

}

RenderViewport::~RenderViewport()
{

}

View* RenderViewport::AllocateView()
{
	m_pView = make_unique<View>();
	return m_pView.get();
}