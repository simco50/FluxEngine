#include "stdafx.h"
#include "Camera.h"

#include "Scenegraph/GameObject.h"
#include "Core/Components/Transform.h"
#include "Math/SimpleMath.h"
#include "Rendering/Core/Graphics.h"

Camera::Camera(InputEngine* pInput, Graphics* pGraphics):
	m_pInput(pInput), m_pGraphics(pGraphics)
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
	m_Viewport.Left = m_VpX * m_pGraphics->GetWindowWidth();
	m_Viewport.Top = m_VpY * m_pGraphics->GetWindowHeight();
	m_Viewport.Right = m_VpWidth;
	m_Viewport.Bottom = m_VpHeight;
}

void Camera::Initialize()
{
	UpdateViewport();
}

void Camera::Update()
{
	XMMATRIX projection, view, viewInv, viewProjInv;
	if(m_Perspective)
		projection = XMMatrixPerspectiveFovLH(m_FoV * (XM_PI / 180.0f), m_Viewport.GetWidth() / m_Viewport.GetHeight(), m_NearPlane, m_FarPlane);
	else
	{
		float viewWidth = m_Size * m_Viewport.GetWidth() / m_Viewport.GetHeight();
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

	UpdateViewport();
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
	UNREFERENCED_PARAMETER(startPoint);
	UNREFERENCED_PARAMETER(direction);

	POINT mousePos =  m_pInput->GetMousePosition();
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