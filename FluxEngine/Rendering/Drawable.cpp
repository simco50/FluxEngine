#include "stdafx.h"
#include "Drawable.h"
#include "SceneGraph/Scene.h"
#include "Renderer.h"

void Drawable::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);
	pScene->GetRenderer()->AddDrawable(this);
}