#pragma once
class SceneNode;
class Drawable;
class Graphics;
class Renderer;

class Scene
{
public:
	Scene(Graphics* pGraphics);
	~Scene();

	virtual void Initialize();
	virtual void Update();

	void AddChild(SceneNode* pNode);
	Renderer* GetRenderer() const { return m_pRenderer.get(); }
	Camera* GetCamera() const { return m_pCamera; }

	SceneNode* Find(const std::string name);

private:
	unique_ptr<Renderer> m_pRenderer;
	std::vector<SceneNode*> m_pNodes;
	Camera* m_pCamera;
};