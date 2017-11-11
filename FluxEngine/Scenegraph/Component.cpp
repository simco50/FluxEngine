#include "stdafx.h"
#include "Component.h"
#include "SceneNode.h"

Transform* Component::GetTransform()
{
	return m_pNode->GetTransform();
}
