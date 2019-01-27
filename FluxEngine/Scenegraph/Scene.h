#pragma once
#include "SceneGraph/SceneNode.h"

class SceneNode;
class Drawable;
class Graphics;
class Renderer;
class Component;
class Camera;

DECLARE_MULTICAST_DELEGATE(OnSceneUpdateDelegate);

class Scene : public SceneNode
{
	friend class SceneNode;

	FLUX_OBJECT(Scene, SceneNode);

public:
	explicit Scene(Context* pContext);
	virtual ~Scene();

	virtual void Initialize();
	void Update();

	Renderer* GetRenderer() const { return m_pRenderer; }
	Camera* GetCamera() const;

	SceneNode* FindNode(const std::string& name);
	const std::vector<SceneNode*>& GetNodes() const { return m_Nodes; }

	virtual void OnSceneSet(Scene* pScene) override;

	OnSceneUpdateDelegate& OnSceneUpdate() { return m_OnSceneUpdate; }

	SceneNode* PickNode(const Ray& ray);

private:
	void TrackChild(SceneNode* pNode);

	Renderer* m_pRenderer;
	std::vector<SceneNode*> m_Nodes;

	OnSceneUpdateDelegate m_OnSceneUpdate;
};