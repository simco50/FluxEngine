#pragma once

class Graphics;
class Shader;
class ShaderVariation;
class ImmediateUI;
class InputEngine;
class FreeCamera;
class Mesh;
class Texture;
class Scene;

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
	std::unique_ptr<Scene> m_pScene;
	std::unique_ptr<Mesh> m_pMeshFilter;

	//Window variables
	HINSTANCE m_hInstance = nullptr;

	Shader* m_pShader = nullptr;
	ShaderVariation* m_pVertexShader = nullptr;
	ShaderVariation* m_pPixelShader = nullptr;
	std::unique_ptr<Texture> m_pDiffuseTexture;

	Graphics* m_pGraphics = nullptr;
	std::unique_ptr<ImmediateUI> m_pImmediateUI;
	std::unique_ptr<InputEngine> m_pInput;

	float m_DeltaTime = 0;
	Color m_Color = Color(1, 1, 1, 1);
	Vector3 m_LightDirection = Vector3(-0.577f, -0.577f, 0.577f);

	int m_IndexCount = -1;

	FreeCamera* m_pCamera = nullptr;
};
