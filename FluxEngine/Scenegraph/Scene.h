#pragma once
#include "SceneGraph\SceneNode.h"

class SceneNode;
class Drawable;
class Graphics;
class Renderer;
class Component;
class Camera;

class Scene : public SceneNode
{
	FLUX_OBJECT(Scene, SceneNode);

public:
	Scene(Graphics* pGraphics);
	virtual ~Scene();

	virtual void Initialize();
	virtual void Update();

	void AddChild(SceneNode* pNode);
	Renderer* GetRenderer() const { return m_pRenderer.get(); }
	Camera* GetCamera() const;

	SceneNode* FindNode(const std::string& name);

	virtual void OnSceneSet(Scene* pScene) override;

private:
	unique_ptr<Renderer> m_pRenderer;
	std::vector<SceneNode*> m_pNodes;
};