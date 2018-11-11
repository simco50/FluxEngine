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
	: Component(pContext), m_Viewport(FloatRect(0.0f, 0.0f, 1.0f, 1.0f))
{
	m_Projection = DirectX::XMMatrixIdentity();
	m_View = DirectX::XMMatrixIdentity();
	m_ViewInverse = DirectX::XMMatrixIdentity();
	m_ViewProjection = DirectX::XMMatrixIdentity();
	m_ViewProjectionInverse = DirectX::XMMatrixIdentity();

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
	float renderTargetWidth = m_pRenderTarget ? (float)m_pRenderTarget->GetParentTexture()->GetWidth() : (float)m_pGraphics->GetWindowWidth();
	float renderTargetHeight = m_pRenderTarget ? (float)m_pRenderTarget->GetParentTexture()->GetHeight() : (float)m_pGraphics->GetWindowHeight();

	FloatRect rect;
	rect.Left = m_Viewport.Left * renderTargetWidth;
	rect.Top = m_Viewport.Top * renderTargetHeight;
	rect.Right = m_Viewport.Right * renderTargetWidth;
	rect.Bottom = m_Viewport.Bottom * renderTargetHeight;
	return rect;
}

RenderTarget* Camera::GetDepthStencil()
{
	if (m_pRenderTarget == nullptr)
	{
		return nullptr;
	}
	if (m_pDepthStencil == nullptr)
	{
		m_pDepthStencil = std::make_unique<Texture2D>(m_pContext);
		m_pDepthStencil->SetSize(m_pRenderTarget->GetParentTexture()->GetWidth(), m_pRenderTarget->GetParentTexture()->GetHeight(), DXGI_FORMAT_R24G8_TYPELESS, TextureUsage::DEPTHSTENCILBUFFER, m_pRenderTarget->GetParentTexture()->GetMultiSample(), nullptr);
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
		m_Projection = DirectX::XMMatrixPerspectiveFovLH(m_FoV * (Math::PI / 180.0f), viewportWidth / viewportHeight, m_NearPlane, m_FarPlane);
	}
	else
	{
		float viewWidth = m_Size * viewportWidth / viewportHeight;
		float viewHeight = m_Size;
		m_Projection = DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, m_NearPlane, m_FarPlane);
	}

	m_View = XMMatrixLookAtLH(
		pNode->GetWorldPosition(),
		pNode->GetWorldPosition() + pNode->GetForward(),
		pNode->GetUp());

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
	m_View.Invert(m_ViewInverse);
	m_ViewProjection = m_View * m_Projection;
	m_ViewProjection.Invert(m_ViewProjectionInverse);
}

void Camera::UpdateFrustum()
{
	BoundingFrustum::CreateFromMatrix(m_Frustum, m_Projection);
	m_Frustum.Transform(m_Frustum, m_ViewInverse);
}

void Camera::SetFOW(float fov)
{
	m_FoV = fov;
	OnMarkedDirty(m_pNode);
}

void Camera::SetViewport(float x, float y, float width, float height)
{
	m_Viewport.Left = x;
	m_Viewport.Top = y;
	m_Viewport.Right = width + x;
	m_Viewport.Bottom = height + y;
	OnMarkedDirty(m_pNode);
}

void Camera::SetClippingPlanes(float nearPlane, float farPlane)
{
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
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
		nearPoint = Vector3::Transform(Vector3(ndc.x, ndc.y, 0), m_ViewProjectionInverse);
		farPoint = Vector3::Transform(Vector3(ndc.x, ndc.y, 1), m_ViewProjectionInverse);
		ray.position = Vector3(nearPoint.x, nearPoint.y, nearPoint.z);

		ray.direction = farPoint - nearPoint;
		ray.direction.Normalize();
	}
	return ray;
}

void Camera::SetNearPlane(float nearPlane)
{
	m_NearPlane = nearPlane;
	OnMarkedDirty(m_pNode);
}

void Camera::SetFarPlane(float farPlane)
{
	m_FarPlane = farPlane;
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
	unchanged &= ImGui::SliderFloat("Near Plane", &m_NearPlane, 0.001f, 1000.0f);
	unchanged &= ImGui::SliderFloat("Far Plane", &m_FarPlane, 10.0f, 100000.0f);
	if (unchanged == false)
	{
		m_pNode->MarkDirty();
	}
}