#pragma once
#include "SceneGraph/Component.h"

class MeshRenderer;
class InputEngine;
class Graphics;

class Camera : public Component
{
public:
	Camera(InputEngine* pInput, Graphics* pGraphics);
	virtual ~Camera();

	const Matrix& GetView() const { return m_View; }
	const Matrix& GetViewInverse() const { return m_ViewInverse; }
	const Matrix& GetViewProjection() const { return m_ViewProjection; }
	const Matrix& GetViewProjectionInverse() const { return m_ViewProjectionInverse; }
	const Matrix& GetProjection() const { return m_Projection; }

	void SetViewport(float x, float y, float width, float height);
	const FloatRect& GetViewport() const { return m_Viewport; }
	void SetClippingPlanes(const float nearPlane, const float farPlane);

	void GetMouseRay(Vector3& startPoint, Vector3& direction);

	void SetOrthographic(bool orthographic) { m_Perspective = !orthographic; }
	void SetOrthographicSize(float size) { m_Size = size; }

	float GetNearPlane() const { return m_NearPlane; }
	float GetFarPlane() const { return m_FarPlane; }

	const BoundingFrustum& GetFrustrum() const { return m_Frustum; }

protected:
	void OnSceneSet(Scene* pScene) override;
	void Update() override;
private:
	InputEngine* m_pInput = nullptr;
	Graphics* m_pGraphics = nullptr;

	float m_Size = 50.0f;
	float m_FoV = 60.0f;

	float m_NearPlane = 0.01f;
	float m_FarPlane = 200.0f;

	Matrix m_View;
	Matrix m_ViewInverse;
	Matrix m_ViewProjection;
	Matrix m_ViewProjectionInverse;
	Matrix m_Projection;

	bool m_Perspective = true;

	FloatRect m_Viewport;
	float m_VpX = 0.0f;
	float m_VpY = 0.0f;
	float m_VpWidth = 1.0f;
	float m_VpHeight = 1.0f;

	BoundingFrustum m_Frustum;
};

