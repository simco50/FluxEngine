#pragma once

class FlexSystem;

class FlexMousePicker
{
public:
	FlexMousePicker(GameContext* pGameContext, FlexSystem* pFlexSystem);
	~FlexMousePicker();

	void Update();

private:
	int PickParticle(const Vector3& origin, const Vector3& direction, float& mouseT) const;
	Vector3 m_MousePos;
	float m_MouseMass = 1.0f;
	int m_PickedParticle = -1;
	float m_MouseT = 0.0f;

	FlexSystem* m_pFlexSystem;
	GameContext* m_pGameContext = nullptr;
};

