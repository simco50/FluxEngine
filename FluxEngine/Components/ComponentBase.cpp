#include "stdafx.h"
#include "ComponentBase.h"
#include "../Scenegraph/GameObject.h"

ComponentBase::ComponentBase()
{
}


ComponentBase::~ComponentBase()
{
}

TransformComponent* ComponentBase::GetTransform() const
{
	return m_pGameObject->GetTransform();
}

void ComponentBase::BaseInitialize(GameContext* pGameContext)
{
	m_pGameContext = pGameContext;
	Initialize();
}
