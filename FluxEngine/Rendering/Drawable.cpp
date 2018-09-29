#include "FluxEngine.h"
#include "Drawable.h"
#include "SceneGraph/Scene.h"
#include "Renderer.h"
#include "Scenegraph/SceneNode.h"

void Drawable::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);
	pScene->GetRenderer()->AddDrawable(this);
}

DirectX::BoundingBox Drawable::GetWorldBoundingBox() const
{
	BoundingBox worldBoundingBox;
	m_BoundingBox.Transform(worldBoundingBox, m_pNode->GetWorldMatrix());
	return worldBoundingBox;
}
