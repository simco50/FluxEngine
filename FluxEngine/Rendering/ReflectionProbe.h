#pragma once
#include "Scenegraph/Component.h"

class TextureCube;
class Camera;
class Texture2D;
class Renderer;

enum class CubeMapFace;

enum class ReflectionSlicingMethod
{
	OnePerFrame,
	AllAtOnce,
};

enum class ReflectionUpdateMode
{
	OnStart,
	OnUpdate,
	Manual,
};

class ReflectionProbe : public Component
{
	FLUX_OBJECT(ReflectionProbe, Component)

public:
	ReflectionProbe(Context* pContext);
	~ReflectionProbe();

	void Capture();
	void Capture(const CubeMapFace face);
	void SetUpdateMode(const ReflectionUpdateMode mode) { m_UpdateMode = mode; }
	void SetResolution(const int resolution);
	void SetTimeSlicingMethod(const ReflectionSlicingMethod method) { m_SlicingMethod = method; }
	TextureCube* GetTexture() const { return m_pCubeTexture.get(); }

protected:
	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnSceneRemoved() override;

private:
	void OnRender();
	void ExecuteRender();

	Renderer* m_pRenderer = nullptr;

	ReflectionUpdateMode m_UpdateMode = ReflectionUpdateMode::Manual;
	int m_Resolution = 512;
	DelegateHandle m_PreRenderHandle;
	ReflectionSlicingMethod m_SlicingMethod = ReflectionSlicingMethod::OnePerFrame;
	std::unique_ptr<TextureCube> m_pCubeTexture;

	std::array<std::unique_ptr<Camera>, 6> m_Cameras;
	float m_FarClip = 100.0f;
	float m_NearClip = 0.1f;

	int m_CurrentFace = 0;
	bool m_Finished = false;
};