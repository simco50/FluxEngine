#include "stdafx.h"
#include "UI_Element.h"


UI_Element::UI_Element()
{
}


UI_Element::~UI_Element()
{
}

void UI_Element::SetContext(GameContext* pGameContext)
{
	m_pGameContext = pGameContext;
}
