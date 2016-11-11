#include "stdafx.h"
#include "UI_Checkbox.h"
#include "../../Graphics/Texture.h"
#include "../../UI/SpriteRenderer.h"
#include "../../UI/TextRenderer.h"

UI_Checkbox::UI_Checkbox(Vector2 position, bool* pValue, const wstring& name) :
	m_pValue(pValue),
	m_Name(name)
{
	m_Position = position;
	if (pValue == nullptr)
		m_pValue = &m_FallbackValue;

	m_pTexture = ResourceManager::Load<Texture>(L"./Resources/Textures/UI/Checkbox.png");
	m_pCheckmarkTexture = ResourceManager::Load<Texture>(L"./Resources/Textures/UI/SliderHandle.png");
}


UI_Checkbox::~UI_Checkbox()
{
}

void UI_Checkbox::Update()
{
	if (m_pGameContext->Scene->Input->IsMouseButtonDown(VK_LBUTTON))
	{
		if (m_MouseDown == false && IsOnCheckbox())
			*m_pValue = !*m_pValue;
		m_MouseDown = true;

	}
	else
		m_MouseDown = false;

	SpriteRenderer::GetInstance()->DrawImmediate(m_pTexture->GetResourceView(), m_Position, Vector4(1, 1, 1, 1), Vector2(0.5f, 0.5f));
	if(*m_pValue)
		SpriteRenderer::GetInstance()->DrawImmediate(m_pCheckmarkTexture->GetResourceView(), m_Position, Vector4(1, 1, 1, 1), Vector2(0.5f, 0.5f));
	if(m_Name.length() > 0)
		TextRenderer::GetInstance()->DrawText(ResourceManager::Load<SpriteFont>(L"./Resources/Fonts/Consolas_22.fnt"), m_Name, m_Position + Vector2(15.0f, -10.0f));
}

bool UI_Checkbox::IsOnCheckbox() const
{
	POINT mp = m_pGameContext->Scene->Input->GetMousePosition();
	Vector2 hp = m_Position;
	if (mp.x < hp.x - m_pTexture->GetWidth() / 2.0f) return false;
	if (mp.x > hp.x + m_pTexture->GetWidth() / 2.0f) return false;

	if (mp.y < hp.y - m_pTexture->GetHeight() / 2.0f) return false;
	if (mp.y > hp.y + m_pTexture->GetHeight() / 2.0f) return false;

	return true;
}
