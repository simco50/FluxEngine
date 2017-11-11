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
	bool RemoveDrawable(Drawable* pDrawable);
	void AddCamera(Camera* pCamera);
	bool RemoveCamera(Camera* pCamera);

	Camera* GetCamera(int camIdx) { return m_Cameras[camIdx]; }

private:
	void SetMaterial(Material* pMaterial);
	Material* m_pCurrentMaterial = nullptr;

	Graphics* m_pGraphics;
	std::vector<Drawable*> m_Drawables;
	std::vector<Camera*> m_Cameras;
};