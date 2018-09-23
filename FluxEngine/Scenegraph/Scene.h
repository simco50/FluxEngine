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
	friend class SceneNode;

	FLUX_OBJECT(Scene, SceneNode);

public:
	Scene(Context* pContext);
	virtual ~Scene();

	virtual void Initialize();
	void Update();

	Renderer* GetRenderer() const { return m_pRenderer; }
	Camera* GetCamera() const;

	SceneNode* FindNode(const std::string& name);
	const std::vector<SceneNode*>& GetNodes() const { return m_Nodes; }

	virtual void OnSceneSet(Scene* pScene) override;

	MulticastDelegate<>& OnSceneUpdate() { return m_OnSceneUpdate; }

private:
	void AddChild(SceneNode* pNode);

	Renderer* m_pRenderer;
	std::vector<SceneNode*> m_Nodes;

	MulticastDelegate<> m_OnSceneUpdate;
};