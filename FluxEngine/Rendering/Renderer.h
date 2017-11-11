#pragma once

class Graphics;
class Drawable;
class Material;

class Renderer
{
public:

	Renderer(Graphics* pGraphics);
	~Renderer();

	void Draw();

	void AddDrawable(Drawable* pDrawable);

private:
	void SetMaterial(Material* pMaterial);
	Material* m_pCurrentMaterial = nullptr;

	Graphics* m_pGraphics;
	std::vector<Drawable*> m_Drawables;
};