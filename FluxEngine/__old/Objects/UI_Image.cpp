#include "stdafx.h"
#include "UI_Image.h"
#include "../../UI/SpriteRenderer.h"
#include "../../Graphics/Texture.h"

UI_Image::UI_Image(const wstring& path, Vector2 position)
{
	m_pTexture = ResourceManager::Load<Texture>(path);
	m_Position = position;
}

UI_Image::~UI_Image()
{
}

void UI_Image::Update()
{
	SpriteRenderer::GetInstance()->DrawImmediate(m_pTexture->GetResourceView(), m_Position);
}
