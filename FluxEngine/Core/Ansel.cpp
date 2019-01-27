#include "FluxEngine.h"
#include "Ansel.h"

#include "AnselSDK.h"
#include "Rendering/Core/Graphics.h"
#include "Rendering/Camera/Camera.h"
#include "Rendering/Core/RenderTarget.h"
#include "Rendering/Core/Texture.h"
#include "Scenegraph/SceneNode.h"
#include "Math/MathTypes.h"
#include "Rendering/Renderer.h"
#include "FluxCore.h"
#include "Rendering/PostProcessing.h"

Ansel::Ansel(Context* pContext)
	: Subsystem(pContext)
{
	if (ansel::isAnselAvailable())
	{
		Graphics* pGraphics = GetSubsystem<Graphics>();
		m_pRenderer = GetSubsystem<Renderer>();
		ansel::Configuration config;
		config.fovType = ansel::kVerticalFov;
		config.isCameraOffcenteredProjectionSupported = true;
		config.isCameraRotationSupported = true;
		config.isCameraTranslationSupported = true;
		config.isCameraFovSupported = true;
		config.gameWindowHandle = pGraphics->GetWindow();
		config.titleNameUtf8 = pGraphics->GetWindowTitle().c_str();
		config.userPointer = this;
		config.up = { 0, 1, 0 };
		config.right = { 1, 0, 0 };
		config.forward = { 0, 0, 1 };
		config.translationalSpeedInWorldUnitsPerSecond = 50;
		config.rotationalSpeedInDegreesPerSecond = 50;

		config.startSessionCallback = [](ansel::SessionConfiguration& configuration, void* pData) { return (ansel::StartSessionStatus)static_cast<Ansel*>(pData)->SessionStarted(configuration); };
		config.stopSessionCallback = [](void* pData) { static_cast<Ansel*>(pData)->SessionStopped(); };
		config.startCaptureCallback = [](const ansel::CaptureConfiguration& configuration, void* pData) { static_cast<Ansel*>(pData)->CaptureStarted(configuration); };
		config.stopCaptureCallback = [](void* pData) { static_cast<Ansel*>(pData)->CaptureStopped(); };
	
		if (!ansel::setConfiguration(config))
		{
			m_IsAvailable = true;
		}
	}
}

void Ansel::Update()
{
	if (m_pCamera && m_InSession == false)
	{
		return;
	}

	ansel::Camera cam;
	cam.fov = m_FoV;
	cam.aspectRatio = m_Width / m_Height;
	cam.farPlane = m_View.FarPlane;
	cam.nearPlane = m_View.NearPlane;
	cam.position = *reinterpret_cast<nv::Vec3*>(&m_Position);
	cam.rotation = *reinterpret_cast<nv::Quat*>(&m_Rotation);
	cam.projectionOffsetX = 0;
	cam.projectionOffsetY = 0;

	ansel::updateCamera(cam);

	m_Position = Vector3(cam.position.x, cam.position.y, cam.position.z);
	m_Rotation = Quaternion(cam.rotation.x, cam.rotation.y, cam.rotation.z, cam.rotation.w);

	m_View.FarPlane = cam.farPlane;
	m_View.NearPlane = cam.nearPlane;
	Matrix projection = Math::CreatePerspectiveMatrix(cam.fov * Math::ToRadians, cam.aspectRatio, cam.nearPlane, cam.farPlane);
	projection._31 = -cam.projectionOffsetX;
	projection._32 = -cam.projectionOffsetY;
	Matrix rotation = Matrix::CreateFromQuaternion(*reinterpret_cast<Quaternion*>(&cam.rotation));
	Matrix translation = Matrix::CreateTranslation(cam.position.x, cam.position.y, cam.position.z);
	m_View.ViewInverseMatrix = rotation * translation;
	m_View.ViewInverseMatrix.Invert(m_View.ViewMatrix);
	m_View.ViewProjectionMatrix = m_View.ViewMatrix * projection;
	BoundingFrustum::CreateFromMatrix(m_View.Frustum, projection);
	m_View.Frustum.Transform(m_View.Frustum, m_View.ViewInverseMatrix);

	m_pRenderer->QueueView(&m_View);
}

int Ansel::SessionStarted(ansel::SessionConfiguration& configuration)
{
	if (m_IsAvailable == false || m_pCamera == nullptr)
	{
		return ansel::StartSessionStatus::kDisallowed;
	}

	PostProcessing* pPostProcessing = m_pCamera->GetComponent<PostProcessing>();
	if (pPostProcessing)
	{
		m_WasPostprocessingEnabled = pPostProcessing->IsEnabled();
		pPostProcessing->SetEnabled(false);
	}
	else
	{
		m_WasPostprocessingEnabled = false;
	}

	m_InSession = true;
	configuration;

	m_View = m_pCamera->GetViewData();
	m_FoV = m_pCamera->GetFoV();
	m_Width = (float)m_pCamera->GetRenderTarget()->GetParentTexture()->GetWidth();
	m_Height = (float)m_pCamera->GetRenderTarget()->GetParentTexture()->GetHeight();

	SceneNode* pNode = m_pCamera->GetNode();
	m_Position = pNode->GetPosition();
	m_Rotation = pNode->GetRotation();

	m_pCamera->SetEnabled(false);

	return ansel::StartSessionStatus::kAllowed;
}

void Ansel::SessionStopped()
{
	if (FluxCore::IsExiting() == false)
	{
		m_pCamera->SetEnabled(true);

		if (m_WasPostprocessingEnabled)
		{
			PostProcessing* pPostProcessing = m_pCamera->GetComponent<PostProcessing>();
			pPostProcessing->SetEnabled(true);
		}
	}
	m_InSession = false;
}

void Ansel::CaptureStarted(const ansel::CaptureConfiguration& configuration)
{
	configuration;
}

void Ansel::CaptureStopped()
{

}