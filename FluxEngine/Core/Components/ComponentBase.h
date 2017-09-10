#pragma once
class GameObject;
class Transform;

class ComponentBase
{
public:
	ComponentBase();
	virtual ~ComponentBase();

	Transform* GetTransform() const;

protected:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Render() {}

	GameContext* m_pGameContext = nullptr;
	GameObject* m_pGameObject = nullptr;

private:

	friend class GameObject;
	friend class ShadowMapper;
	void BaseInitialize(GameContext* pGameContext);
};