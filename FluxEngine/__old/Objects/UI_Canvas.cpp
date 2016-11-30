#include "stdafx.h"
#include "UI_Canvas.h"
#include "UI_Element.h"

UI_Canvas::UI_Canvas(GameContext* pGameContext)
{
	m_pGameContext = pGameContext;
}

UI_Canvas::~UI_Canvas()
{
	for(UI_Element*& pElement : m_Elements)
	{
		delete pElement;
		pElement = nullptr;
	}
}

void UI_Canvas::Update()
{
	if (m_IsActive == false)
		return;
	for(UI_Element* pElement : m_Elements)
	{
		if(pElement->IsActive())
			pElement->Update();
	}
}

void UI_Canvas::AddElement(UI_Element* pElement)
{
#ifdef _DEBUG
	for (size_t i = 0; i < m_Elements.size(); i++)
	{
		if(pElement == m_Elements[i])
		{
			DebugLog::Log(L"UI_Canvas::AddElement() > Element already exists in canvas", LogType::WARNING);
			return;
		}
	}
#endif
	pElement->SetContext(m_pGameContext);
	m_Elements.push_back(pElement);
}

void UI_Canvas::ToggleActive()
{
	m_IsActive = !m_IsActive;
}
