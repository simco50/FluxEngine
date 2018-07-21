#include "FluxEngine.h"
#include "Camera.h"

#include "Scenegraph/Scene.h"
#include "Scenegraph/SceneNode.h"
#include "SceneGraph/Transform.h"

#include "Rendering/Core/Graphics.h"
#include "Rendering/Renderer.h"

#include "Audio/AudioListener.h"
#include "Physics/PhysX/PhysicsScene.h"
#include "Input/InputEngine.h"
#include "../Core/RenderTarget.h"
#include "../Core/Texture.h"

Camera::Camera(Context* pContext):
	Component(pContext), m_Viewport(FloatRect(0.0f, 0.0f, 1.0f, 1.0f))
{
	m_Projection = XMMatrixIdentity();
	m_View = XMMatrixIdentity();
	m_ViewInverse = XMMatrixIdentity();
	m_ViewProjection = XMMatrixIdentity();
	m_ViewProjectionInverse = XMMatrixIdentity();

	m_pGraphics = GetSubsystem<Graphics>();
}

Camera::~Camera()
{
}

void Camera::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);
	pScene->GetRenderer()->AddCamera(this);
	OnMarkedDirty(GetTransform());
}

FloatRect Camera::GetAbsoluteViewport() const
{
	FloatRect rect = m_Viewport;

	float renderTargetWidth = m_pRenderTarget ? (float)m_pRenderTarget->GetParentTexture()->GetWidth() : (float)m_pGraphics->GetWindowWidth();
	float renderTargetHeight = m_pRenderTarget ? (float)m_pRenderTarget->GetParentTexture()->GetHeight() : (float)m_pGraphics->GetWindowHeight();
	
	rect.Left = m_Viewport.Left * renderTargetWidth;
	rect.Top = m_Viewport.Top * renderTargetHeight;
	rect.Right = m_Viewport.Right * renderTargetWidth;
	rect.Bottom = m_Viewport.Bottom * renderTargetHeight;

	return rect;
}

void Camera::OnMarkedDirty(const Transform* transform)
{
	FloatRect absolute = GetAbsoluteViewport();
	float viewportWidth = absolute.GetWidth();
	float viewportHeight = absolute.GetHeight();

	if (m_Perspective)
	{
		m_Projection = XMMatrixPerspectiveFovLH(m_FoV * (XM_PI / 180.0f), viewportWidth / viewportHeight, m_NearPlane, m_FarPlane);
	}
	else
	{
		float viewWidth = m_Size * viewportWidth / viewportHeight;
		float viewHeight = m_Size;
		m_Projection = XMMatrixOrthographicLH(viewWidth, viewHeight, m_NearPlane, m_FarPlane);
	}

	m_View = XMMatrixLookAtLH(
		transform->GetWorldPosition(), 
		transform->GetWorldPosition() + transform->GetForward(), 
		XMLoadFloat3(&transform->GetUp()));

	m_View.Invert(m_ViewInverse);

	m_ViewProjection = m_View * m_Projection;
	m_ViewProjection.Invert(m_ViewProjectionInverse);

	UpdateFrustum();
}

void Camera::SetProjection(const Matrix& projection)
{
	m_Projection = projection;
	m_ViewProjection = m_View * m_Projection;
	m_ViewProjection.Invert(m_ViewProjectionInverse);
}

void Camera::SetView(const Matrix& view)
{
	m_View = view;
	m_ViewProjection = m_View * m_Projection;
	m_ViewProjection.Invert(m_ViewProjectionInverse);
}

void Camera::UpdateFrustum()
{
	BoundingFrustum::CreateFromMatrix(m_Frustum, m_Projection);
	m_Frustum.Transform(m_Frustum, m_ViewInverse);
}

void Camera::SetFOW(const float fov)
{
	m_FoV = fov;
	OnMarkedDirty(GetTransform());
}

void Camera::SetViewport(float x, float y, float width, float height)
{
	m_Viewport.Left = x;
	m_Viewport.Top = y;
	m_Viewport.Right = width + x;
	m_Viewport.Bottom = height + y;
	OnMarkedDirty(GetTransform());
}

void Camera::SetClippingPlanes(const float nearPlane, const float farPlane)
{
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
	OnMarkedDirty(GetTransform());
}

void Camera::GetMouseRay(Vector3& startPoint, Vector3& direction) const
{
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
		nearPoint = Vector3::Transform(Vector3(ndc.x, ndc.y, 0), m_ViewProjectionInverse);
		farPoint = Vector3::Transform(Vector3(ndc.x, ndc.y, 1), m_ViewProjectionInverse);
		startPoint = Vector3(nearPoint.x, nearPoint.y, nearPoint.z);

		direction = farPoint - nearPoint;
		direction.Normalize();
	}
}

void Camera::SetNearPlane(const float nearPlane)
{
	m_NearPlane = nearPlane;
	OnMarkedDirty(GetTransform());
}

void Camera::SetFarPlane(const float farPlane)
{
	m_FarPlane = farPlane;
	OnMarkedDirty(GetTransform());
}

bool Camera::Raycast(RaycastResult& result) const
{
	result = RaycastResult();

	if (m_pScene == nullptr)
		return false;
	PhysicsScene* pPhysicsScene = m_pScene->GetComponent<PhysicsScene>();
	if (pPhysicsScene == nullptr)
		return false;

	Vector3 rayStart, rayDir;
	GetMouseRay(rayStart, rayDir);

	return pPhysicsScene->Raycast(
		rayStart,
		rayDir,
		result
	);
}
