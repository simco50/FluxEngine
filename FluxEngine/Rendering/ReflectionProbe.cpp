#include "FluxEngine.h"
#include "ReflectionProbe.h"
#include "Camera/Camera.h"
#include "Core/TextureCube.h"
#include "Scenegraph/Transform.h"
#include "Renderer.h"
#include "Core/Texture2D.h"

ReflectionProbe::ReflectionProbe(Context* pContext) :
	Component(pContext)
{

}

ReflectionProbe::~ReflectionProbe()
{

}

void ReflectionProbe::Capture(const CubeMapFace face)
{
	Matrix projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, m_NearClip, m_FarClip);
	std::unique_ptr<Camera>& pCamera = m_Cameras[(int)face];
	pCamera->SetProjection(projection);

	Vector3 position = GetTransform()->GetWorldPosition();

	switch (face)
	{
	case CubeMapFace::POSITIVE_X:
		pCamera->SetView(Matrix::CreateLookAt(position, position + Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)));
		break;
	case CubeMapFace::NEGATIVE_X:
		pCamera->SetView(Matrix::CreateLookAt(position, position + Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)));
		break;
	case CubeMapFace::POSITIVE_Y:
		pCamera->SetView(Matrix::CreateLookAt(position, position + Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f)));
		break;
	case CubeMapFace::NEGATIVE_Y:
		pCamera->SetView(Matrix::CreateLookAt(position, position + Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)));
		break;
	case CubeMapFace::POSITIVE_Z:
		pCamera->SetView(Matrix::CreateLookAt(position, position + Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)));
		break;
	case CubeMapFace::NEGATIVE_Z:
		pCamera->SetView(Matrix::CreateLookAt(position, position + Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f)));
		break;
	case CubeMapFace::MAX:
	default:
		break;
	}
	pCamera->UpdateFrustum();
	m_pRenderer->QueueCamera(pCamera.get());
}

void ReflectionProbe::Capture()
{
	for (int i = 0; i < (int)CubeMapFace::MAX; ++i)
	{
		Capture((CubeMapFace)i);
	}
}

void ReflectionProbe::SetResolution(const int resolution)
{
	if (resolution != m_Resolution)
	{
		m_Resolution = resolution;
		m_pCubeTexture->SetSize(resolution, resolution, DXGI_FORMAT_R8G8B8A8_UNORM, TextureUsage::RENDERTARGET, 1, nullptr);
	}
}

void ReflectionProbe::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);

	m_pRenderer = GetSubsystem<Renderer>();

	m_pCubeTexture = std::make_unique<TextureCube>(m_pContext);
	m_pCubeTexture->SetSize(m_Resolution, m_Resolution, DXGI_FORMAT_R8G8B8A8_UNORM, TextureUsage::RENDERTARGET, 1, nullptr);

	for (size_t i = 0; i < m_Cameras.size(); ++i)
	{
		std::unique_ptr<Camera>& pCamera = m_Cameras[i];
		if (pCamera == nullptr)
		{
			pCamera = std::make_unique<Camera>(m_pContext);
		}
		pCamera->SetRenderTarget(m_pCubeTexture->GetRenderTarget((CubeMapFace)i));
	}

	m_PreRenderHandle = m_pRenderer->OnPreRender().AddRaw(this, &ReflectionProbe::OnRender);
}

void ReflectionProbe::OnSceneRemoved()
{
	Component::OnSceneRemoved();
	m_Cameras = {};
	m_pCubeTexture.reset();
	m_pRenderer->OnPreRender().Remove(m_PreRenderHandle);
}

void ReflectionProbe::OnRender()
{
	switch (m_UpdateMode)
	{
	case ReflectionUpdateMode::OnStart:
		if (m_Finished == false)
		{
			ExecuteRender();
		}
		break;
	case ReflectionUpdateMode::OnUpdate:
		ExecuteRender();
		break;
	case ReflectionUpdateMode::Manual:
	default:
		//Do nothing
		break;
	}
}

void ReflectionProbe::ExecuteRender()
{
	switch (m_SlicingMethod)
	{
	case ReflectionSlicingMethod::OnePerFrame:
		Capture((CubeMapFace)m_CurrentFace);
		++m_CurrentFace;
		if (m_CurrentFace >= (int)CubeMapFace::MAX)
		{
			m_CurrentFace = 0;
			m_Finished = true;
		}
		break;
	case ReflectionSlicingMethod::AllAtOnce:
		Capture();
		m_Finished = true;
		break;
	}
}
