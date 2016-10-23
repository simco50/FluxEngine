#pragma once
class GameObject;
class TransformComponent;

class ComponentBase
{
public:
	ComponentBase();
	virtual ~ComponentBase();

	TransformComponent* GetTransform() const;

protected:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;

	GameContext* m_pGameContext = nullptr;
	GameObject* m_pGameObject = nullptr;

private:

	friend class GameObject;
	friend class ShadowMapper;
	void BaseInitialize(GameContext* pGameContext);
};