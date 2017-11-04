#pragma once
#include "Core/Components/ComponentBase.h"

class MeshRenderer;
class InputEngine;
class Graphics;

class Camera : public ComponentBase
{
public:
	Camera(InputEngine* pInput, Graphics* pGraphics);
	virtual ~Camera();

	const XMFLOAT4X4& GetView() const { return m_View; }
	const XMFLOAT4X4& GetViewInverse() const { return m_ViewInverse; }
	const XMFLOAT4X4& GetViewProjection() const { return m_ViewProjection; }
	const XMFLOAT4X4& GetViewProjectionInverse() const { return m_ViewProjectionInverse; }
	const XMFLOAT4X4& GetProjection() const { return m_Projection; }

	void SetViewport(float x, float y, float width, float height);
	void UpdateViewport();
	const FloatRect& GetViewport() const { return m_Viewport; }
	void SetClippingPlanes(const float nearPlane, const float farPlane);

	void GetMouseRay(Vector3& startPoint, Vector3& direction);

	void SetOrthographic(bool orthographic) { m_Perspective = !orthographic; }
	void SetOrthographicSize(float size) { m_Size = size; }

	float GetNearPlane() const { return m_NearPlane; }
	float GetFarPlane() const { return m_FarPlane; }

protected:
	void Initialize() override;
	void Update() override;
private:
	InputEngine* m_pInput = nullptr;
	Graphics* m_pGraphics = nullptr;

	float m_Size = 50.0f;
	float m_FoV = 60.0f;

	float m_NearPlane = 1.0f;
	float m_FarPlane = 200.0f;

	XMFLOAT4X4 m_View;
	XMFLOAT4X4 m_ViewInverse;
	XMFLOAT4X4 m_ViewProjection;
	XMFLOAT4X4 m_ViewProjectionInverse;
	XMFLOAT4X4 m_Projection;

	bool m_Perspective = true;

	FloatRect m_Viewport;
	float m_VpX = 0.0f;
	float m_VpY = 0.0f;
	float m_VpWidth = 1.0f;
	float m_VpHeight = 1.0f;

	BoundingFrustum m_Frustum;
};

