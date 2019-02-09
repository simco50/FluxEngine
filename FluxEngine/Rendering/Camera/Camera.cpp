#include "FluxEngine.h"
#include "Camera.h"

#include "Scenegraph/Scene.h"
#include "Scenegraph/SceneNode.h"

#include "Rendering/Core/Graphics.h"
#include "Rendering/Renderer.h"

#include "Audio/AudioListener.h"
#include "Physics/PhysX/PhysicsScene.h"
#include "Input/InputEngine.h"
#include "../Core/RenderTarget.h"
#include "../Core/Texture.h"
#include "../Core/Texture2D.h"

Camera::Camera(Context* pContext)
	: Component(pContext), m_View({})
{
	m_pGraphics = GetSubsystem<Graphics>();
}

Camera::~Camera()
{
}

void Camera::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);
	pScene->GetRenderer()->AddCamera(this);
	OnMarkedDirty(m_pNode);
}

FloatRect Camera::GetAbsoluteViewport() const
{
	float renderTargetWidth = m_View.pRenderTarget ? (float)m_View.pRenderTarget->GetParentTexture()->GetWidth() : (float)m_pGraphics->GetWindowWidth();
	float renderTargetHeight = m_View.pRenderTarget ? (float)m_View.pRenderTarget->GetParentTexture()->GetHeight() : (float)m_pGraphics->GetWindowHeight();

	FloatRect rect;
	rect.Left = m_View.Viewport.Left * renderTargetWidth;
	rect.Top = m_View.Viewport.Top * renderTargetHeight;
	rect.Right = m_View.Viewport.Right * renderTargetWidth;
	rect.Bottom = m_View.Viewport.Bottom * renderTargetHeight;
	return rect;
}

RenderTarget* Camera::GetRenderTarget() const
{
	if (m_View.pRenderTarget)
	{
		return m_View.pRenderTarget;
	}
	return m_pGraphics->GetDefaultRenderTarget()->GetRenderTarget();
}

RenderTarget* Camera::GetDepthStencil()
{
	if (m_View.pRenderTarget == nullptr)
	{
		return nullptr;
	}
	if (m_pDepthStencil == nullptr)
	{
		m_pDepthStencil = std::make_unique<Texture2D>(m_pContext);
		m_pDepthStencil->SetSize(m_View.pRenderTarget->GetParentTexture()->GetWidth(), m_View.pRenderTarget->GetParentTexture()->GetHeight(), DXGI_FORMAT_R24G8_TYPELESS, TextureUsage::DEPTHSTENCILBUFFER, m_View.pRenderTarget->GetParentTexture()->GetMultiSample(), nullptr);
	}
	return m_pDepthStencil->GetRenderTarget();
}

void Camera::OnMarkedDirty(const SceneNode* pNode)
{
	FloatRect absolute = GetAbsoluteViewport();
	float viewportWidth = absolute.GetWidth();
	float viewportHeight = absolute.GetHeight();

	if (m_Perspective)
	{
		m_ProjectionMatrix = Math::CreatePerspectiveMatrix(m_FoV * Math::ToRadians, viewportWidth / viewportHeight, m_View.NearPlane, m_View.FarPlane);
	}
	else
	{
		float viewWidth = m_Size * viewportWidth / viewportHeight;
		float viewHeight = m_Size;
		m_ProjectionMatrix = Math::CreateOrthographicMatrix(viewWidth, viewHeight, m_View.NearPlane, m_View.FarPlane);
	}

	m_View.ViewInverseMatrix = pNode->GetWorldMatrix();
	m_View.ViewInverseMatrix.Invert(m_View.ViewMatrix);

	m_View.ViewProjectionMatrix = m_View.ViewMatrix * m_ProjectionMatrix;

	UpdateFrustum();
}

void Camera::UpdateFrustum()
{
	BoundingFrustum::CreateFromMatrix(m_View.Frustum, m_ProjectionMatrix);
	m_View.Frustum.Transform(m_View.Frustum, m_View.ViewInverseMatrix);
}

void Camera::SetFOW(float fov)
{
	m_FoV = fov;
	OnMarkedDirty(m_pNode);
}

void Camera::SetViewport(float x, float y, float width, float height)
{
	m_View.Viewport.Left = x;
	m_View.Viewport.Top = y;
	m_View.Viewport.Right = width + x;
	m_View.Viewport.Bottom = height + y;
	OnMarkedDirty(m_pNode);

	m_ViewportChangedEvent.Broadcast(m_View.Viewport);
}

void Camera::SetClippingPlanes(float nearPlane, float farPlane)
{
	m_View.NearPlane = nearPlane;
	m_View.FarPlane = farPlane;
	OnMarkedDirty(m_pNode);
}

Ray Camera::GetMouseRay() const
{
	Ray ray;
	InputEngine* input = GetSubsystem<InputEngine>();
	if (input)
	{
		FloatRect absolute = GetAbsoluteViewport();

		Vector2 mousePos = input->GetMousePosition();
		Vector2 ndc;
		float hw = absolute.GetWidth() / 2.0f;
		float hh = absolute.GetHeight() / 2.0f;
		ndc.x = (mousePos.x - hw) / hw + absolute.Left;
		ndc.y = (hh - mousePos.y) / hh + absolute.Top;

		Vector3 nearPoint, farPoint;
		Matrix viewProjInverse;
		m_View.ViewProjectionMatrix.Invert(viewProjInverse);
		nearPoint = Vector3::Transform(Vector3(ndc.x, ndc.y, 0), viewProjInverse);
		farPoint = Vector3::Transform(Vector3(ndc.x, ndc.y, 1), viewProjInverse);
		ray.position = Vector3(nearPoint.x, nearPoint.y, nearPoint.z);

		ray.direction = farPoint - nearPoint;
		ray.direction.Normalize();
	}
	return ray;
}

void Camera::SetOrthographic(bool orthographic)
{
	m_Perspective = !orthographic;
	OnMarkedDirty(m_pNode);
}

void Camera::SetOrthographicSize(float size)
{
	m_Size = size;
	OnMarkedDirty(m_pNode);
}

void Camera::SetNearPlane(float nearPlane)
{
	m_View.NearPlane = nearPlane;
	OnMarkedDirty(m_pNode);
}

void Camera::SetFarPlane(float farPlane)
{
	m_View.FarPlane = farPlane;
	OnMarkedDirty(m_pNode);
}

bool Camera::Raycast(RaycastResult& result) const
{
	result = RaycastResult();

	if (m_pScene == nullptr)
	{
		return false;
	}
	PhysicsScene* pPhysicsScene = m_pScene->GetComponent<PhysicsScene>();
	if (pPhysicsScene == nullptr)
	{
		return false;
	}
	return pPhysicsScene->Raycast(GetMouseRay(), result);
}

void Camera::CreateUI()
{
	bool unchanged = true;
	unchanged &= ImGui::SliderFloat("FoV", &m_FoV, 1, 179);
	unchanged &= ImGui::SliderFloat("Orthographic Size", &m_Size, 1, 200);
	unchanged &= ImGui::Checkbox("Perspective", &m_Perspective);
	unchanged &= ImGui::SliderFloat("Near Plane", &m_View.NearPlane, 0.001f, 1000.0f);
	unchanged &= ImGui::SliderFloat("Far Plane", &m_View.FarPlane, 10.0f, 100000.0f);
	if (unchanged == false)
	{
		m_pNode->MarkDirty();
	}
}