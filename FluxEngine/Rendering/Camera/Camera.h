#pragma once

#include "SceneGraph/Component.h"
#include "Rendering/View.h"

class Graphics;
class RenderTarget;
class Texture2D;
struct RaycastResult;

DECLARE_MULTICAST_DELEGATE(CameraViewportChangedDelegate, const FloatRect&);

class Camera : public Component
{
	FLUX_OBJECT(Camera, Component)

public:
	explicit Camera(Context* pContext);
	virtual ~Camera();

	void UpdateFrustum();

	void SetFOW(float fov);
	void SetViewport(float x, float y, float width, float height);
	FloatRect GetViewport() const { return GetAbsoluteViewport(); }
	void SetClippingPlanes(float nearPlane, float farPlane);

	Ray GetMouseRay() const;

	void SetOrthographic(bool orthographic);
	void SetOrthographicSize(float size);

	void SetNearPlane(float nearPlane);
	void SetFarPlane(float farPlane);

	bool Raycast(RaycastResult& result) const;

	void SetRenderTarget(RenderTarget* pRenderTarget) { m_View.pRenderTarget = pRenderTarget; }
	void SetRenderOrder(const int order) { m_Order = order; }

	void SetClearFlags(const ClearFlags& flags) { m_View.ClearFlags = flags; }
	void SetClearColor(const Color& color) { m_View.ClearColor = color; }

	CameraViewportChangedDelegate& ViewportChanged() { return m_ViewportChangedEvent; }

	float GetFoV() const { return m_FoV; }
	const View& GetViewData() const { return m_View; }
	RenderTarget* GetRenderTarget() const;
	RenderTarget* GetDepthStencil();
	int GetRenderOrder() const { return m_Order; }

	virtual void OnMarkedDirty(const SceneNode* pNode) override;

	virtual void CreateUI() override;

protected:
	void OnSceneSet(Scene* pScene) override;
private:
	FloatRect GetAbsoluteViewport() const;

	Graphics* m_pGraphics = nullptr;

	float m_Size = 50.0f;
	float m_FoV = 60.0f;

	View m_View;
	bool m_Perspective = true;
	Matrix m_ProjectionMatrix = Matrix::Identity;

	CameraViewportChangedDelegate m_ViewportChangedEvent;
	int m_Order = 0;
	std::unique_ptr<Texture2D> m_pDepthStencil;
};