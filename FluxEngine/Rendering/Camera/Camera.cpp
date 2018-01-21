#include "FluxEngine.h"
#include "Camera.h"

#include "Scenegraph/Scene.h"
#include "Scenegraph/SceneNode.h"

#include "SceneGraph/Transform.h"

#include "Rendering/Core/Graphics.h"
#include "Rendering/Renderer.h"
#include "Audio/AudioListener.h"
#include "Physics/PhysX/PhysicsScene.h"

Camera::Camera(InputEngine* pInput, Graphics* pGraphics):
	m_pInput(pInput), m_pGraphics(pGraphics)
{
	m_Projection = XMMatrixIdentity();
	m_View = XMMatrixIdentity();
	m_ViewInverse = XMMatrixIdentity();
	m_ViewProjection = XMMatrixIdentity();
	m_ViewProjectionInverse = XMMatrixIdentity();
}

Camera::~Camera()
{
}

void Camera::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);
	pScene->GetRenderer()->AddCamera(this);
}

void Camera::OnMarkedDirty(const Transform* transform)
{
	if (m_Perspective)
		m_Projection = XMMatrixPerspectiveFovLH(m_FoV * (XM_PI / 180.0f), m_Viewport.GetWidth() / m_Viewport.GetHeight(), m_NearPlane, m_FarPlane);
	else
	{
		float viewWidth = m_Size * m_Viewport.GetWidth() / m_Viewport.GetHeight();
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

	BoundingFrustum::CreateFromMatrix(m_Frustum, m_Projection);
	m_Frustum.Transform(m_Frustum, m_ViewInverse);
}

void Camera::SetViewport(float x, float y, float width, float height)
{
	m_Viewport.Left = x;
	m_Viewport.Top = y;
	m_Viewport.Right = width + x;
	m_Viewport.Bottom = height + y;
}

void Camera::SetClippingPlanes(const float nearPlane, const float farPlane)
{
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
}

void Camera::GetMouseRay(Vector3& startPoint, Vector3& direction) const
{
	POINT mousePos = m_pInput->GetMousePosition();
	Vector2 ndc;
	float hw = m_Viewport.GetWidth() / 2.0f;
	float hh = m_Viewport.GetHeight() / 2.0f;
	ndc.x = (static_cast<float>(mousePos.x) - hw) / hw + m_Viewport.Left;
	ndc.y = (hh - static_cast<float>(mousePos.y)) / hh + m_Viewport.Top;

	Vector3 nearPoint, farPoint;
	nearPoint = Vector3::Transform(Vector3(ndc.x, ndc.y, 0), m_ViewProjectionInverse);
	farPoint = Vector3::Transform(Vector3(ndc.x, ndc.y, 1), m_ViewProjectionInverse);
	startPoint = Vector3(nearPoint.x, nearPoint.y, nearPoint.z);

	direction = farPoint - nearPoint;
	direction.Normalize();
}

RaycastResult Camera::Raycast() const
{
	if (m_pScene == nullptr)
		return RaycastResult();
	PhysicsScene* pPhysicsScene = m_pScene->GetComponent<PhysicsScene>();
	if (pPhysicsScene == nullptr)
		return RaycastResult();

	Vector3 rayStart, rayDir;
	GetMouseRay(rayStart, rayDir);

	return pPhysicsScene->Raycast(
		rayStart,
		rayDir
	);
}
