#pragma once
#include "UI_Element.h"

#include "../../Graphics/Texture.h"
#include "../../UI/SpriteRenderer.h"
#include "../../UI/TextRenderer.h"
#include <iomanip>

template<class T>
class UI_Slider : public UI_Element
{
public:
	UI_Slider(Vector2 position, T* pValue, T minimum = 0.0f, T maximum = 1.0f, const wstring& name = L"") :
		m_pValue(pValue),
		m_Minimum(minimum),
		m_Maximum(maximum)
	{
		m_Position = position;

		m_pSliderTexture = ResourceManager::Load<Texture>(L"./Resources/Textures/UI/Slider.png");
		m_pHandleTexture = ResourceManager::Load<Texture>(L"./Resources/Textures/UI/SliderHandle.png");

		if (pValue == nullptr)
			m_pValue = &m_FallbackValue;
		m_LerpValue = ((float)*m_pValue - minimum) / (maximum - minimum);
		if (*m_pValue > maximum)
			*m_pValue = maximum;
		else if (*m_pValue < minimum)
			*m_pValue = minimum;

		m_OriginalValue = *m_pValue;

		m_Name = name;
	}
	~UI_Slider() {}

	void Update()
	{
		if (m_pGameContext->Scene->Input->IsMouseButtonDown(VK_LBUTTON))
		{
			if (m_MouseDown == false)
			{
				m_MouseDown = true;
				if (IsOnHandle())
					m_Drag = true;
			}
		}
		else
		{
			m_MouseDown = false;
			m_Drag = false;
		}

		if (m_Drag)
			Drag();

		if (m_pGameContext->Scene->Input->IsMouseButtonDown(VK_RBUTTON))
		{
			if (IsOnHandle())
				Reset();
		}

		Vector2 handlePos = m_Position + Vector2(m_pSliderTexture->GetWidth() * m_LerpValue, 0);
		SpriteRenderer::GetInstance()->DrawImmediate(m_pSliderTexture->GetResourceView(), m_Position, Vector4(1, 1, 1, 1), Vector2(0.0f, 0.5f));
		SpriteRenderer::GetInstance()->DrawImmediate(m_pHandleTexture->GetResourceView(), handlePos, Vector4(1, 1, 1, 1), Vector2(0.5f, 0.5f));

		if (m_Name.length() > 0)
			TextRenderer::GetInstance()->DrawText(ResourceManager::Load<SpriteFont>(L"./Resources/Fonts/Consolas_22.fnt"), m_Name, m_Position + Vector2(0.0f, -30.0f));
		wstringstream stream;
		stream << setprecision(2) << *m_pValue;
		TextRenderer::GetInstance()->DrawText(ResourceManager::Load<SpriteFont>(L"./Resources/Fonts/Consolas_22.fnt"), stream.str(), m_Position + Vector2(m_pSliderTexture->GetWidth() + 5.0f, -m_pSliderTexture->GetHeight() / 4.0f));

	}

	T GetValue()
	{
		return *m_pValue;
	}

private:
	bool IsOnHandle() const
	{
		POINT mp = m_pGameContext->Scene->Input->GetMousePosition();
		Vector2 hp = m_Position + Vector2(m_pSliderTexture->GetWidth() * m_LerpValue, 0);
		if (mp.x < hp.x - m_pHandleTexture->GetWidth() / 2.0f) return false;
		if (mp.x > hp.x + m_pHandleTexture->GetWidth() / 2.0f) return false;

		if (mp.y < hp.y - m_pHandleTexture->GetHeight() / 2.0f) return false;
		if (mp.y > hp.y + m_pHandleTexture->GetHeight() / 2.0f) return false;

		return true;
	}

	void Drag()
	{
		POINT mp = m_pGameContext->Scene->Input->GetMousePosition();
		m_LerpValue = (mp.x - m_Position.x) / m_pSliderTexture->GetWidth();

		if (m_LerpValue > 1.0f)
			m_LerpValue = 1.0f;
		else if (m_LerpValue < 0.0f)
			m_LerpValue = 0.0f;
		*m_pValue = m_Minimum + m_LerpValue * (m_Maximum - m_Minimum);
	}
	void Reset()
	{
		*m_pValue = m_OriginalValue;
		m_LerpValue = ((float)*m_pValue - m_Minimum) / (m_Maximum - m_Minimum);
	}
	T* m_pValue;
	T m_OriginalValue = 0;
	T m_FallbackValue = 0;
	float m_LerpValue;
	T m_Minimum;
	T m_Maximum;
	bool m_MouseDown = false;
	bool m_Drag = true;

	Vector2 m_HandlePosition;

	Texture* m_pHandleTexture = nullptr;
	Texture* m_pSliderTexture = nullptr;
};
