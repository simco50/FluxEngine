#pragma once
#include "UI_Element.h"

class UI_Canvas : public UI_Element
{
public:
	UI_Canvas(GameContext* pGameContext);
	~UI_Canvas();

	void Update();
	void AddElement(UI_Element* pElement);
	void ToggleActive();

private:
	vector<UI_Element*> m_Elements;
};

