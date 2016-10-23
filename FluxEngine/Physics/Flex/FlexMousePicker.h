#pragma once
namespace FlexHelper 
{
	struct FlexData;
}

class FlexMousePicker
{
public:
	FlexMousePicker(GameContext* pGameContext, FlexHelper::FlexData* pFlexData);
	~FlexMousePicker();

	void Update();

private:
	int PickParticle(const Vector3& origin, const Vector3& direction, float& mouseT) const;
	Vector3 m_MousePos;
	float m_MouseMass = 1.0f;
	int m_PickedParticle = -1;
	float m_MouseT = 0.0f;

	FlexHelper::FlexData* m_pFlexData = nullptr;
	GameContext* m_pGameContext = nullptr;
};

