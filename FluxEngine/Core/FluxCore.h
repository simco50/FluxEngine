#pragma once

class Graphics;
class Shader;
class ShaderVariation;
class ImmediateUI;
class InputEngine;
class FreeCamera;
class MeshFilter;
class Texture;

class FluxCore
{
public:
	FluxCore();
	virtual ~FluxCore();

	DELETE_COPY(FluxCore)

	int Run(HINSTANCE hInstance);
	void GameLoop();

	void UpdatePerFrameParameters();
	void UpdatePerObjectParameters();
	void UpdatePerViewParameters();

	void RenderUI();

	void InitGame();

private:
	//Window variables
	HINSTANCE m_hInstance = nullptr;

	Shader* m_pShader = nullptr;
	ShaderVariation* m_pVertexShader = nullptr;
	ShaderVariation* m_pPixelShader = nullptr;
	MeshFilter* m_pMeshFilter = nullptr;
	Texture* m_pDiffuseTexture = nullptr;

	Graphics* m_pGraphics = nullptr;
	unique_ptr<ImmediateUI> m_pImmediateUI;
	unique_ptr<InputEngine> m_pInput;

	float m_DeltaTime = 0;
	Color m_Color = Color(1, 1, 1, 1);
	Vector3 m_LightDirection = Vector3(-0.577f, -0.577f, 0.577f);

	int m_IndexCount = -1;

	unique_ptr<FreeCamera> m_pCamera;
};
