#include "stdafx.h"
#include "UI_Button.h"
#include "../../Graphics/Texture.h"
#include "../../UI/SpriteRenderer.h"
#include "../../UI/TextRenderer.h"

UI_Button::UI_Button(Vector2 position, function<void(void)> callback, const wstring& name):
m_Callback(callback)
{
	m_Position = position;
	m_Name = name;

	m_pDefaultTexture = ResourceManager::Load<Texture>(L"./Resources/Textures/UI/ButtonDefault.png");
	m_pMouseOverTexture = ResourceManager::Load<Texture>(L"./Resources/Textures/UI/ButtonHover.png");
}


UI_Button::~UI_Button()
{
}

void UI_Button::Update()
{
	bool onButton = IsOnButton();

	if (m_pGameContext->Scene->Input->IsMouseButtonDown(VK_LBUTTON))
	{
		if (m_MouseDown == false)
		{
			m_MouseDown = true;
			if(onButton)
				m_Callback();
		}
	}
	else
		m_MouseDown = false;

	if (onButton)
		SpriteRenderer::GetInstance()->DrawImmediate(m_pMouseOverTexture->GetResourceView(), m_Position);

	else
		SpriteRenderer::GetInstance()->DrawImmediate(m_pDefaultTexture->GetResourceView(), m_Position);
	if(m_Name.length() != 0)
		TextRenderer::GetInstance()->DrawText(ResourceManager::Load<SpriteFont>(L"./Resources/Fonts/Consolas_22.fnt"), m_Name, m_Position + Vector2(5.0f, 5.0f));
}

bool UI_Button::IsOnButton() const
{
	POINT mp = m_pGameContext->Scene->Input->GetMousePosition();
	Vector2 p = m_Position;
	if (mp.x < p.x) return false;
	if (mp.x > p.x + m_pDefaultTexture->GetWidth()) return false;

	if (mp.y < p.y) return false;
	if (mp.y > p.y + m_pDefaultTexture->GetHeight()) return false;

	return true;
}
