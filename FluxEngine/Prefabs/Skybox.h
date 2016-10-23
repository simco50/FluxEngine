#pragma once
#include "../Scenegraph/GameObject.h"

class SkyboxMaterial;
class Texture;

class Skybox : public GameObject
{
public:
	Skybox();
	~Skybox();

	void Initialize() override;
	void Update() override;
	void Render() override;

	void SetTexture(Texture* pTexture);

private:
	unique_ptr<SkyboxMaterial> m_pMaterial;
};

