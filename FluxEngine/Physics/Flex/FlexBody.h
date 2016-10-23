#pragma once
#include "..\Scenegraph\GameObject.h"

struct FlexSolver;
class Texture;
namespace FlexHelper
{
	struct FlexData;
	struct FlexMeshInstance;
}

class FlexBody : public GameObject
{
public:
	FlexBody(const wstring& filePath, FlexHelper::FlexData* pFlexData);
	virtual ~FlexBody();

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;

	void SetPosition(const Vector3& position);

protected:
	void AddBodyFlexData(FlexExtAsset* pAsset, const int phase);

	FlexHelper::FlexMeshInstance* m_pMeshInstance = nullptr;
	FlexHelper::FlexData* m_pFlexData;
	wstring m_FilePath;
};

