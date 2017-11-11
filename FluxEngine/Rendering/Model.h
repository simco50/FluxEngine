#pragma once
#include "Rendering/Drawable.h"

class Mesh;

class Model : public Drawable
{
public:
	Model();
	~Model();

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnNodeSet(SceneNode* pNode) override;

	void SetMesh(Mesh* pMesh);
	void SetMaterial(Material* pMaterial);
	void SetMaterial(int index, Material* pMaterial);
		
private:
	Mesh* m_pMesh = nullptr;
};