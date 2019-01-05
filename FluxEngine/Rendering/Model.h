#pragma once
#include "Rendering/Drawable.h"

class Mesh;
class Skeleton;

class Model : public Drawable
{
	FLUX_OBJECT(Model, Drawable)

public:
	Model(Context* pContext);
	virtual ~Model();

	virtual void SetMesh(Mesh* pMesh);
	Mesh* GetMesh() const { return m_pMesh; }
	void SetMaterial(Material* pMaterial);
	void SetMaterial(int index, Material* pMaterial);

	virtual void CreateUI() override;

protected:
	Mesh* m_pMesh = nullptr;
};