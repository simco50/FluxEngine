#pragma once
#include "..\Scenegraph\GameObject.h"

class Texture;
class FlexSystem;

namespace FlexHelper
{
	struct FlexMeshInstance;
}

class FlexBody : public GameObject
{
public:
	FlexBody(const wstring& filePath, FlexSystem* pFlexSystem);
	virtual ~FlexBody();

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;

	void SetPosition(const Vector3& position);

protected:
	void AddBodyFlexData(FlexExtAsset* pAsset, const int phase);

	FlexHelper::FlexMeshInstance* m_pMeshInstance = nullptr;
	FlexSystem* m_pFlexSystem;
	wstring m_FilePath;
};

