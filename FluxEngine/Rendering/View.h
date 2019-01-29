#pragma once

class RenderTarget;

struct View
{
	Matrix ViewMatrix = Matrix::Identity;
	Matrix ViewProjectionMatrix = Matrix::Identity;
	Matrix ViewInverseMatrix = Matrix::Identity;
	float NearPlane = 0.1f;
	float FarPlane = 100.0f;
	RenderTarget* pRenderTarget = nullptr;
	RenderTarget* pDepthStencil = nullptr;
	FloatRect Viewport = FloatRect(0, 0, 1, 1);
	BoundingFrustum Frustum{};
	Color ClearColor = Color(0.2f, 0.2f, 0.2f, 1.0f);
	ClearFlags ClearFlags = ClearFlags::All;
};