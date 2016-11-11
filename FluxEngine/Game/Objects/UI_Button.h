#pragma once

#include "UI_Element.h"

class Texture;

class UI_Button : public UI_Element
{
public:
	UI_Button(Vector2 position, function<void(void)> callback, const wstring& name = L"");
	~UI_Button();

	void Update();

private:
	bool IsOnButton() const;
	bool m_MouseDown = false;

	Texture* m_pDefaultTexture = nullptr;
	Texture* m_pMouseOverTexture = nullptr;

	function<void(void)> m_Callback;
};

