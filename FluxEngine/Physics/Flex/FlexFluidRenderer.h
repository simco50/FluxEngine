#pragma once
#include "../Scenegraph/GameObject.h"

class FlexSystem;
class MeshFilter;
class QuadRenderer;
class FluidMaterial;

class FlexFluidRenderer : public GameObject
{
public:
	FlexFluidRenderer(FlexSystem* pFlexSystem);
	~FlexFluidRenderer();

	void Initialize();
	void Update();
	void Render();

	void SetMesh(const wstring& path);
	void SetColor(const Vector4& color) { m_Color = color; }
	void SetScale(const float scale) { m_Scale = scale; }

private:
	void LoadShaderVariables();
	void CreateInputLayout();
	void CreateIndexBuffer();
	void CreateVertexBuffer();
	void CreateInstanceData();

	void CreateQuadVertexBuffer();
	void CreateQuadInputLayout();
	void RenderSpheres();
	void RenderQuad();

	struct DebugParticle
	{
		Vector3 Position;
		int Phase;
	};
	vector<DebugParticle> m_Particles;

	//Flex systems
	int m_ParticleCount = 0;
	FlexSystem* m_pFlexSystem;

	//D3D Rendering objects
	//Sphere rendering
	Unique_COM<ID3D11Buffer> m_pVertexBuffer;
	Unique_COM<ID3D11Buffer> m_pInstanceBuffer;
	Unique_COM<ID3D11Buffer> m_pIndexBuffer;
	ID3DX11EffectTechnique* m_pTechnique = nullptr;
	ID3DX11Effect* m_pEffect = nullptr;
	Unique_COM<ID3D11InputLayout> m_pInputLayout;
	RenderTarget* m_pRenderTarget = nullptr;
	//Quad rendering
	Unique_COM<ID3D11Buffer> m_pQuadVertexBuffer;
	Unique_COM<ID3D11InputLayout> m_pQuadInputLayout;
	ID3DX11EffectTechnique* m_pQuadTechnique = nullptr;
	ID3DX11Effect* m_pQuadEffect = nullptr;
	MeshFilter* m_pMeshFilter = nullptr;

	int m_MaxParticles = -1;

	//Shader variables
	//Sphere rendering
	ID3DX11EffectMatrixVariable* m_pVPVar = nullptr;
	ID3DX11EffectScalarVariable* m_pScaleVar = nullptr;
	//Quad rendering
	ID3DX11EffectVectorVariable* m_pLightDirectionVar = nullptr;
	ID3DX11EffectMatrixVariable* m_pViewInverseVar = nullptr;
	ID3DX11EffectMatrixVariable* m_pProjectionInverseVar = nullptr;
	ID3DX11EffectShaderResourceVariable* m_pDepthMapVariable = nullptr;

	//Properties
	float m_Scale = 1.0f;
	Vector4 m_Color;
};

