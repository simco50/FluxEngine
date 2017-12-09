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
class SceneNode;
class Material;
class ParticleSystem;

class FluxCore
{
public:
	FluxCore();
	virtual ~FluxCore();

	DELETE_COPY(FluxCore)

	int Run(HINSTANCE hInstance);
	void GameLoop();
	void RenderUI();
	void InitGame();

private:
	std::unique_ptr<Scene> m_pScene;
	std::unique_ptr<Mesh> m_pMeshFilter;

	//Window variables
	HINSTANCE m_hInstance = nullptr;

	unique_ptr<Material> m_pMaterial;

	Graphics* m_pGraphics = nullptr;
	std::unique_ptr<ImmediateUI> m_pImmediateUI;
	std::unique_ptr<InputEngine> m_pInput;
	std::unique_ptr<ParticleSystem> m_pParticleSystem;

	float m_DeltaTime = 0;
	Color m_Color = Color(1, 1, 1, 1);

	int m_IndexCount = -1;
	FreeCamera* m_pCamera = nullptr;
	SceneNode* m_pNode = nullptr;
};
