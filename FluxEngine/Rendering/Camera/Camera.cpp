#include "stdafx.h"
#include "Camera.h"

#include "Scenegraph/GameObject.h"
#include "Core/Components/Transform.h"
#include "Math/SimpleMath.h"
#include "Rendering/MeshRenderer.h"

Camera::Camera()
{
	XMStoreFloat4x4(&m_Projection, XMMatrixIdentity());
	XMStoreFloat4x4(&m_View, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewInverse, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjection, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjectionInverse, XMMatrixIdentity());
}

Camera::~Camera()
{
}

void Camera::UpdateViewport()
{
	m_Viewport.Height = m_VpHeight * m_pGameContext->Engine->GameSettings.Height;
	m_Viewport.Width = m_VpWidth * m_pGameContext->Engine->GameSettings.Width;
	m_Viewport.MaxDepth = 1.0f;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.TopLeftX = m_VpX * m_pGameContext->Engine->GameSettings.Width;
	m_Viewport.TopLeftY = m_VpY * m_pGameContext->Engine->GameSettings.Height;
}

void Camera::Initialize()
{
	UpdateViewport();
}

void Camera::Update()
{
	XMMATRIX projection, view, viewInv, viewProjInv;
	if(m_Perspective)
		projection = XMMatrixPerspectiveFovLH(m_FoV * (XM_PI / 180.0f), m_Viewport.Width / m_Viewport.Height, m_NearPlane, m_FarPlane);
	else
	{
		float viewWidth = m_Size * m_Viewport.Width / m_Viewport.Height;
		float viewHeight = m_Size;
		projection = XMMatrixOrthographicLH(viewWidth, viewHeight, m_NearPlane, m_FarPlane);
	}
	
	XMVECTOR worldPos = XMLoadFloat3(&m_pGameObject->GetTransform()->GetWorldPosition());
	XMVECTOR lookAt = XMLoadFloat3(&m_pGameObject->GetTransform()->GetForward());
	XMVECTOR upDirection = XMLoadFloat3(&m_pGameObject->GetTransform()->GetUp());
	view = XMMatrixLookAtLH(worldPos, worldPos + lookAt, upDirection);
	viewInv = XMMatrixInverse(nullptr, view);
	viewProjInv = XMMatrixInverse(nullptr, view * projection);

	XMStoreFloat4x4(&m_View, view);
	XMStoreFloat4x4(&m_ViewInverse, viewInv);
	XMStoreFloat4x4(&m_Projection, projection);
	XMStoreFloat4x4(&m_ViewProjection, view * projection);
	XMStoreFloat4x4(&m_ViewProjectionInverse, viewProjInv);

	if (m_FrustumCulling)
	{
		BoundingFrustum frustrum(projection);
		frustrum.Transform(m_Frustum, viewInv);
	}
}

void Camera::SetViewport(float x, float y, float width, float height)
{
	m_VpX = x;
	m_VpY = y;
	m_VpWidth = width;
	m_VpHeight = height;
	UpdateViewport();
}

void Camera::SetClippingPlanes(const float nearPlane, const float farPlane)
{
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
}

void Camera::GetMouseRay(Vector3& startPoint, Vector3& direction)
{
	POINT mousePos = InputEngine::Instance().GetMousePosition();
	Vector2 ndc;
	float hw = m_Viewport.Width / 2.0f;
	float hh = m_Viewport.Height / 2.0f;
	ndc.x = (static_cast<float>(mousePos.x) - hw) / hw + m_Viewport.TopLeftX;
	ndc.y = (hh - static_cast<float>(mousePos.y)) / hh + m_Viewport.TopLeftY;

	Vector3 nearPoint, farPoint;
	nearPoint = Vector3::Transform(Vector3(ndc.x, ndc.y, 0), m_ViewProjectionInverse);
	farPoint = Vector3::Transform(Vector3(ndc.x, ndc.y, 1), m_ViewProjectionInverse);
	startPoint = Vector3(nearPoint.x, nearPoint.y, nearPoint.z);

	direction = farPoint - nearPoint;
	direction.Normalize();
}

bool Camera::IsInFrustum(MeshRenderer* pObj) const
{
	if (m_FrustumCulling == false)
		return true;
	if (pObj == nullptr)
		return true;
	return m_Frustum.Intersects(pObj->GetBoundingBox());
}