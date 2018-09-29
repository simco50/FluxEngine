#pragma once

#include "SceneGraph/Component.h"
#include "../Core/GraphicsDefines.h"

class Graphics;
class RenderTarget;
class Texture2D;
struct RaycastResult;

class Camera : public Component
{
	FLUX_OBJECT(Camera, Component)

public:
	Camera(Context* pContext);
	virtual ~Camera();

	const Matrix& GetView() const { return m_View; }
	const Matrix& GetViewInverse() const { return m_ViewInverse; }
	const Matrix& GetViewProjection() const { return m_ViewProjection; }
	const Matrix& GetViewProjectionInverse() const { return m_ViewProjectionInverse; }
	const Matrix& GetProjection() const { return m_Projection; }

	void SetProjection(const Matrix& projection);
	void SetView(const Matrix& view);
	void UpdateFrustum();

	void SetFOW(float fov);
	void SetViewport(float x, float y, float width, float height);
	FloatRect GetViewport() const { return GetAbsoluteViewport(); }
	void SetClippingPlanes(float nearPlane, float farPlane);

	void GetMouseRay(Vector3& startPoint, Vector3& direction) const;

	void SetOrthographic(bool orthographic) { m_Perspective = !orthographic; }
	void SetOrthographicSize(float size) { m_Size = size; }

	float GetNearPlane() const { return m_NearPlane; }
	float GetFarPlane() const { return m_FarPlane; }

	void SetNearPlane(float nearPlane);
	void SetFarPlane(float farPlane);

	const BoundingFrustum& GetFrustum() const { return m_Frustum; }

	bool Raycast(RaycastResult& result) const;

	void SetRenderTarget(RenderTarget* pRenderTarget) { m_pRenderTarget = pRenderTarget; }
	void SetRenderOrder(const int order) { m_Order = order; }

	void SetClearFlags(const ClearFlags& flags) { m_ClearFlags = flags; }
	void SetClearColor(const Color& color) { m_ClearColor = color; }

	RenderTarget* GetRenderTarget() const { return m_pRenderTarget; }
	RenderTarget* GetDepthStencil();
	int GetRenderOrder() const { return m_Order; }
	const ClearFlags& GetClearFlags() const { return m_ClearFlags; }
	const Color& GetClearColor() const { return m_ClearColor; }

	virtual void OnMarkedDirty(const SceneNode* pNode) override;

protected:
	void OnSceneSet(Scene* pScene) override;
private:
	FloatRect GetAbsoluteViewport() const;

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
	int m_Order = 0;

	BoundingFrustum m_Frustum;
	RenderTarget* m_pRenderTarget = nullptr;
	std::unique_ptr<Texture2D> m_pDepthStencil;
	Color m_ClearColor = Color(0.2f, 0.2f, 0.2f, 1.0f);
	ClearFlags m_ClearFlags = ClearFlags::All;
};