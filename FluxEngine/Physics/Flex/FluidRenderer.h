#pragma once
#include "../../Scenegraph/GameObject.h"

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

	void SetScale(const float scale) { m_Scale = scale; }

private:
	void CreateRenderTarget();
	void LoadShaderVariables();
	void CreateInputLayouts();
	void CreateVertexBuffers();

	//Flex systems
	int m_ParticleCount = 0;
	FlexSystem* m_pFlexSystem;

	//D3D Rendering objects
	Unique_COM<ID3D11Buffer> m_pVertexBuffer;
	ID3DX11EffectTechnique* m_pTechnique = nullptr;
	ID3DX11Effect* m_pEffect = nullptr;
	Unique_COM<ID3D11InputLayout> m_pInputLayout;

	RenderTarget* m_pParticleRenderTarget = nullptr;
	Unique_COM<ID3D11Buffer> m_pQuadVertexBuffer;
	ID3DX11EffectTechnique* m_pFluidTechnique = nullptr;
	ID3DX11Effect* m_pFluidEffect = nullptr;

	Unique_COM<ID3D11InputLayout> m_pQuadInputLayout;

	int m_MaxParticles = -1;

	//Shader variables
	ID3DX11EffectMatrixVariable* m_pVPVar = nullptr;
	ID3DX11EffectScalarVariable* m_pScaleVar = nullptr;
	ID3DX11EffectVectorVariable* m_pLightDirectionVar = nullptr;
	ID3DX11EffectMatrixVariable* m_pViewInverseVar = nullptr;

	ID3DX11EffectShaderResourceVariable* m_pDepthMap = nullptr;

	//Properties
	float m_Scale = 1.0f;
	Vector4 m_Color;
};

