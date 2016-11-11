#pragma once
class UI_Element
{
public:
	UI_Element();
	virtual ~UI_Element();

	virtual void Update() = 0;

	bool IsActive() const { return m_IsActive; }
	void SetContext(GameContext* pGameContext);

protected:
	Vector2 m_Position;
	wstring m_Name;

	bool m_IsActive = true;
	GameContext* m_pGameContext = nullptr;
};

