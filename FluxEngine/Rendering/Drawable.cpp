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

void Drawable::CreateUI()
{
	ImGui::Checkbox("Draw", &m_Draw);
	ImGui::Checkbox("Cull", &m_Cull);
	ImGui::Text("Bounding Box");
	ImGui::InputFloat3("Center", &m_BoundingBox.Center.x);
	ImGui::InputFloat3("Extents", &m_BoundingBox.Extents.x);
}
