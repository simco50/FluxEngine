#pragma once
#include "../../Scenegraph/GameObject.h"

class FlexSystem;
class MeshFilter;

class FlexDebugRenderer : public GameObject
{
public:
	FlexDebugRenderer(FlexSystem* pFlexSystem);
	~FlexDebugRenderer();

	void Initialize();
	void Update();
	void Render();

	void SetMesh(const string& path);
	void SetColor(const Vector4& color) { m_Color = color; }
	void SetScale(const float scale) { m_Scale = scale; }

	void EnableDebugging(const bool enabled) { m_Enabled = enabled; }
	void ToggleDebugging() { m_Enabled = !m_Enabled; }

private:
	void LoadShaderVariables();
	void UpdateShaderVariables();
	void CreateInputLayout();
	void CreateIndexBuffer();
	void CreateVertexBuffer();
	void CreateInstanceData();

	struct DebugParticle
	{
		Vector3 Position;
		int Phase;
	};
	vector<DebugParticle> m_Particles;

	bool m_Enabled = false;

	//Flex systems
	int m_ParticleCount = 0;
	FlexSystem* m_pFlexSystem;

	//D3D Rendering objects
	Unique_COM<ID3D11Buffer> m_pVertexBuffer;
	Unique_COM<ID3D11Buffer> m_pInstanceBuffer;
	Unique_COM<ID3D11Buffer> m_pIndexBuffer;
	ID3DX11EffectTechnique* m_pTechnique = nullptr;
	ID3DX11Effect* m_pEffect = nullptr;
	Unique_COM<ID3D11InputLayout> m_pInputLayout;

	MeshFilter* m_pMeshFilter = nullptr;

	int m_MaxParticles = -1;

	//Shader variables
	ID3DX11EffectVectorVariable* m_pLightDirectionVar = nullptr;
	ID3DX11EffectVectorVariable* m_pColorVar = nullptr;
	ID3DX11EffectMatrixVariable* m_pVPVar = nullptr;
	ID3DX11EffectMatrixVariable* m_pViewInverseVar = nullptr;
	ID3DX11EffectScalarVariable* m_pScaleVar = nullptr;

	//Properties
	float m_Scale = 1.0f;
	Vector4 m_Color;
};

