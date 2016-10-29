#include "stdafx.h"
#include "FlexMousePicker.h"
#include "FlexHelper.h"
#include "../../Components/CameraComponent.h"
#include "../../FlexSystem.h"

FlexMousePicker::FlexMousePicker(GameContext* pGameContext, FlexSystem* pFlexSystem): m_pFlexSystem(pFlexSystem), m_pGameContext(pGameContext)
{
	m_pGameContext->Scene->Input->AddInputAction(InputAction(10, Pressed, -1, VK_LBUTTON));
	m_pGameContext->Scene->Input->AddInputAction(InputAction(11, Released, -1, VK_LBUTTON));
}

FlexMousePicker::~FlexMousePicker()
{
}

void FlexMousePicker::Update()
{
	if (m_pGameContext->Scene->Input->IsActionTriggered(10))
	{
		Vector3 origin, direction;
		m_pGameContext->Scene->Cameras[0]->GetMouseRay(origin, direction);
		m_PickedParticle = PickParticle(origin, direction, m_MouseT);
		if (m_PickedParticle != -1)
		{
			DebugLog::LogFormat(LogType::INFO, L"Picked particle %i.", m_PickedParticle);
			m_MousePos = origin + direction * m_MouseT;
			m_MouseMass = m_pFlexSystem->Positions[m_PickedParticle].w;
			m_pFlexSystem->Positions[m_PickedParticle].w = 0.0f;
		}
	}
	if (m_pGameContext->Scene->Input->IsActionTriggered(11))
	{
		if (m_PickedParticle != -1)
		{
			m_pFlexSystem->Positions[m_PickedParticle].w = m_MouseMass;
			m_MouseMass = -1;
			m_PickedParticle = -1;
		}
	}
	if (m_PickedParticle != -1)
	{
		Vector3 origin, direction;
		m_pGameContext->Scene->Cameras[0]->GetMouseRay(origin, direction);
		m_MousePos = origin + direction * m_MouseT;

		Vector3 p = Vector3::Lerp(Vector3(m_pFlexSystem->Positions[m_PickedParticle].x, m_pFlexSystem->Positions[m_PickedParticle].y, m_pFlexSystem->Positions[m_PickedParticle].z), m_MousePos, 10.0f * m_pGameContext->Scene->GameTimer.DeltaTime());
		Vector3 delta = p - Vector3(m_pFlexSystem->Positions[m_PickedParticle].x, m_pFlexSystem->Positions[m_PickedParticle].y, m_pFlexSystem->Positions[m_PickedParticle].z);

		m_pFlexSystem->Positions[m_PickedParticle].x = p.x;
		m_pFlexSystem->Positions[m_PickedParticle].y = p.y;
		m_pFlexSystem->Positions[m_PickedParticle].z = p.z;

		m_pFlexSystem->Velocities[m_PickedParticle] = delta / (1.0f / 60.0f);
	}
}

int FlexMousePicker::PickParticle(const Vector3& origin, const Vector3& direction, float& mouseT) const
{
	float maxDistSq = m_pFlexSystem->Params.mRadius * m_pFlexSystem->Params.mRadius;
	float minT = FLT_MAX;
	int minIndex = -1;
	for (size_t i = 0; i < m_pFlexSystem->Positions.size(); i++)
	{
		Vector3 delta = Vector3(m_pFlexSystem->Positions[i].x, m_pFlexSystem->Positions[i].y, m_pFlexSystem->Positions[i].z) - origin;
		float t = delta.Dot(direction);

		if (t > 0.0f)
		{
			Vector3 perp = delta - t * direction;
			float dSq = perp.LengthSquared();
			if (dSq < maxDistSq && t < minT)
			{
				minT = t;
				minIndex = i;
			}
		}
	}
	mouseT = minT;

	return minIndex;
}
