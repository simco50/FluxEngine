#pragma once
#include "Core\Subsystem.h"

class Graphics;
class VertexBuffer;
class ShaderVariation;
class Camera;

struct VertexElement;

struct DebugLine
{
	DebugLine(const Vector3& start, const Vector3& end, const Color& color) :
		Start(start), End(end), Color(color)
	{}

	Vector3 Start;
	Vector3 End;
	Color Color;
};

struct DebugTriangle
{
	DebugTriangle(const Vector3& a, const Vector3& b, const Vector3& c, const Color& color) :
		A(a), B(b), C(c), Color(color)
	{}

	Vector3 A;
	Vector3 B;
	Vector3 C;
	Color Color;
};

class DebugRenderer : public Subsystem
{
	FLUX_OBJECT(DebugRenderer, Subsystem)

public:
	DebugRenderer(Graphics* pGraphics);
	virtual ~DebugRenderer();

	void Render();
	void EndFrame();

	void SetCamera(Camera* pCamera) { m_pCamera = pCamera; }

	void AddLine(const Vector3& start, const Vector3& end, const Color& color);
	void AddTriangle(const Vector3& a, const Vector3& b, const Vector3& c, const Color& color);
	void AddPolygon(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, const Color& color);
	void AddBoundingBox(const BoundingBox& boundingBox, const Color& color, const bool solid = false);
	void AddBoundingBox(const BoundingBox& boundingBox, const Matrix& transform, const Color& color, const bool solid = false);
	void AddFrustrum(const BoundingFrustum& frustrum, const Color& color);
	void DrawAxisSystem(const Matrix& transform, const float lineLength = 1.0f);

private:

	Graphics* m_pGraphics;
	Camera* m_pCamera = nullptr;
	std::unique_ptr<VertexBuffer> m_pVertexBuffer;
	std::vector<VertexElement> m_ElementDesc;
	ShaderVariation* m_pVertexShader = nullptr;
	ShaderVariation* m_pPixelShader = nullptr;

	int m_LinePrimitives = 0;
	vector<DebugLine> m_Lines;
	int m_TrianglePrimitives = 0;
	vector<DebugTriangle> m_Triangles;
};