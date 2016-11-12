#pragma once
#include "../Scenegraph/GameObject.h"

class FlexSystem;
class MeshFilter;
class QuadRenderer;
class FluidMaterial;

class FluidRenderer : public GameObject
{
public:
	FluidRenderer(FlexSystem* pFlexSystem);
	~FluidRenderer();

	void Initialize();
	void Update();
	void Render();

	void SetMesh(const wstring& path);
	void SetColor(const Vector4& color) { m_Color = color; }
	void SetScale(const float scale) { m_Scale = scale; }

private:
	void LoadShaderVariables();
	void CreateInputLayout();
	void CreateVertexBuffer();

	//Flex systems
	int m_ParticleCount = 0;
	FlexSystem* m_pFlexSystem;

	//D3D Rendering objects
	Unique_COM<ID3D11Buffer> m_pVertexBuffer;
	ID3DX11EffectTechnique* m_pTechnique = nullptr;
	ID3DX11Effect* m_pEffect = nullptr;
	Unique_COM<ID3D11InputLayout> m_pInputLayout;

	int m_MaxParticles = -1;

	//Shader variables
	ID3DX11EffectMatrixVariable* m_pVPVar = nullptr;
	ID3DX11EffectScalarVariable* m_pScaleVar = nullptr;
	//Quad rendering
	ID3DX11EffectVectorVariable* m_pLightDirectionVar = nullptr;
	ID3DX11EffectMatrixVariable* m_pViewInverseVar = nullptr;

	//Properties
	float m_Scale = 1.0f;
	Vector4 m_Color;
};

