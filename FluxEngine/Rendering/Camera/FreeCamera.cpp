#include "FluxEngine.h"
#include "FreeCamera.h"
#include "Camera.h"
#include "Audio/AudioListener.h"
#include "Input/InputEngine.h"
#include "Scenegraph/Scene.h"

FreeCamera::FreeCamera(Context* pContext) :
	FreeObject(pContext)
{
}

FreeCamera::~FreeCamera()
{
}

void FreeCamera::OnSceneSet(Scene* pScene)
{
	FreeObject::OnSceneSet(pScene);

	if (GetSubsystem<AudioEngine>(false))
	{
		CreateComponent<AudioListener>();
	}
	m_pCamera = CreateComponent<Camera>();
}

void FreeCamera::OnSceneRemoved()
{
	FreeObject::OnSceneRemoved();
}