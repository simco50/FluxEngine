#pragma once
class Material;

class MaterialManager
{
public:
	MaterialManager();
	~MaterialManager();

	void Initialize(GameContext* pGameContext);

	int AddMaterial(Material* pMaterial);
	template <typename T>
	T* GetMaterial(int id);
	
private:
	vector<Material*> m_Materials;

	GameContext* m_pGameContext = nullptr;
};

