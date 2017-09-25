#include "stdafx.h"
#include "Renderer.h"
#include "RenderViewport.h"
#include "View.h"

Renderer::Renderer()
{

}

Renderer::~Renderer()
{

}

void Renderer::SetViewport(const unsigned int index, RenderViewport* pViewport)
{
	if(m_Viewports.size() <= index)
		m_Viewports.resize(index + 1);
	m_Viewports[index] = pViewport;
}

void Renderer::Render()
{
	for (unsigned int i = 0; i < m_Viewports.size(); ++i)
	{
		if (m_Viewports[i]->GetView() == nullptr)
			m_Viewports[i]->AllocateView();
		View* pView = m_Viewports[i]->GetView();
		pView->Define(m_Viewports[i]);
		pView->Render();
	}
}