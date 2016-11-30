#pragma once
#include "UI_Element.h"

class Texture;

class UI_Image : public UI_Element
{
public:
	UI_Image(const wstring& path, Vector2 position);
	~UI_Image();

	void Update();

private:
	Texture* m_pTexture;
};

