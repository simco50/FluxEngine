#pragma once
#include "UI_Element.h"

class Texture;

class UI_Checkbox : public UI_Element
{
public:
	UI_Checkbox(Vector2 position, bool* pValue, const wstring& name = L"");
	~UI_Checkbox();

	void Update();

private:
	wstring m_Name;
	bool m_MouseDown = false;
	bool IsOnCheckbox() const;

	Texture* m_pTexture = nullptr;
	Texture* m_pCheckmarkTexture = nullptr;

	bool m_FallbackValue = false;
	bool* m_pValue;
};

