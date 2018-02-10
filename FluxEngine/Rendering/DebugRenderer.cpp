#include "FluxEngine.h"
#include "DebugRenderer.h"
#include "Core\VertexBuffer.h"
#include "Core\Graphics.h"
#include "Core\Shader.h"
#include "Core\DepthStencilState.h"
#include "Core\BlendState.h"
#include "Core\RasterizerState.h"
#include "Camera\Camera.h"
#include "Physics\PhysX\PhysicsScene.h"
#include "Mesh.h"
#include "Geometry.h"

DebugRenderer::DebugRenderer(Context* pContext) :
	Subsystem(pContext)
{
	m_pGraphics = pContext->GetSubsystem<Graphics>();

	m_ElementDesc = 
	{
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::POSITION, 0, false),
		VertexElement(VertexElementType::FLOAT4, VertexElementSemantic::COLOR, 0, false),
	};
	m_pVertexShader = m_pGraphics->GetShader("Resources/Shaders/DebugRenderer.hlsl", ShaderType::VertexShader);
	m_pPixelShader = m_pGraphics->GetShader("Resources/Shaders/DebugRenderer.hlsl", ShaderType::PixelShader);
	m_pVertexBuffer = make_unique<VertexBuffer>(m_pGraphics);
}

DebugRenderer::~DebugRenderer()
{

}

void DebugRenderer::Render()
{
	int totalPrimitives = m_LinePrimitives + m_TrianglePrimitives;

	if (totalPrimitives == 0 || m_pCamera == nullptr)
		return;

	m_pGraphics->SetShader(ShaderType::VertexShader, m_pVertexShader);
	m_pGraphics->SetShader(ShaderType::PixelShader, m_pPixelShader);
	m_pGraphics->SetShader(ShaderType::ComputeShader, nullptr);
	m_pGraphics->SetShader(ShaderType::GeometryShader, nullptr);

	if (totalPrimitives > (int)m_pVertexBuffer->GetVertexCount())
	{
		m_pVertexBuffer->Create(totalPrimitives + 100, m_ElementDesc, true);
	}

	char* pData = (char*)m_pVertexBuffer->Map(true);
	char* pDestination = pData;
	for (const DebugLine& line : m_Lines)
	{
		memcpy(pDestination, &line.Start, sizeof(Vector3));
		pDestination += sizeof(Vector3);
		memcpy(pDestination, &line.ColorStart, sizeof(Color));
		pDestination += sizeof(Color);
		memcpy(pDestination, &line.End, sizeof(Vector3));
		pDestination += sizeof(Vector3);
		memcpy(pDestination, &line.ColorEnd, sizeof(Color));
		pDestination += sizeof(Color);
	}
	for (const DebugTriangle& triangle : m_Triangles)
	{
		memcpy(pDestination, &triangle.A, sizeof(Vector3));
		pDestination += sizeof(Vector3);
		memcpy(pDestination, &triangle.ColorA, sizeof(Color));
		pDestination += sizeof(Color);
		memcpy(pDestination, &triangle.B, sizeof(Vector3));
		pDestination += sizeof(Vector3);
		memcpy(pDestination, &triangle.ColorB, sizeof(Color));
		pDestination += sizeof(Color);
		memcpy(pDestination, &triangle.C, sizeof(Vector3));
		pDestination += sizeof(Vector3);
		memcpy(pDestination, &triangle.ColorC, sizeof(Color));
		pDestination += sizeof(Color);
	}

	m_pVertexBuffer->Unmap();

	m_pGraphics->SetVertexBuffer(m_pVertexBuffer.get());
	m_pGraphics->SetIndexBuffer(nullptr);

	m_pGraphics->GetDepthStencilState()->SetDepthEnabled(true);
	m_pGraphics->GetDepthStencilState()->SetDepthTest(CompareMode::LESSEQUAL);

	m_pGraphics->GetBlendState()->SetColorWrite(ColorWrite::ALL);
	m_pGraphics->GetBlendState()->SetBlendMode(BlendMode::REPLACE, false);

	m_pGraphics->GetRasterizerState()->SetCullMode(CullMode::BACK);

	Matrix projectionMatrix = m_pCamera->GetViewProjection();
	m_pGraphics->SetShaderParameter("cViewProj", &projectionMatrix);

	int start = 0;
	if(m_LinePrimitives != 0)
		m_pGraphics->Draw(PrimitiveType::LINELIST, start, m_LinePrimitives);
	start += m_LinePrimitives;
	if(m_TrianglePrimitives != 0)
		m_pGraphics->Draw(PrimitiveType::TRIANGLELIST, start, m_TrianglePrimitives);
}

void DebugRenderer::EndFrame()
{
	m_LinePrimitives = 0;
	m_Lines.clear();
	m_TrianglePrimitives = 0;
	m_Triangles.clear();
}

void DebugRenderer::AddLine(const Vector3& start, const Vector3& end, const Color& color)
{
	AddLine(start, end, color, color);
}

void DebugRenderer::AddLine(const Vector3& start, const Vector3& end, const Color& colorStart, const Color& colorEnd)
{
	m_Lines.push_back(DebugLine(start, end, colorStart, colorEnd));
	m_LinePrimitives += 2;
}

void DebugRenderer::AddRay(const Vector3& start, const Vector3& direction, const Color& color)
{
	AddLine(start, start + direction, color);
}

void DebugRenderer::AddTriangle(const Vector3& a, const Vector3& b, const Vector3& c, const Color& color, bool solid)
{
	AddTriangle(a, b, c, color, color, color, solid);
}

void DebugRenderer::AddTriangle(const Vector3& a, const Vector3& b, const Vector3& c, const Color& colorA, const Color& colorB, const Color& colorC, bool solid)
{
	if (solid)
	{
		m_Triangles.push_back(DebugTriangle(a, b, c, colorA, colorB, colorC));
		m_TrianglePrimitives += 3;
	}
	else
	{
		AddLine(a, b, colorA);
		AddLine(b, c, colorB);
		AddLine(c, b, colorC);
	}
}

void DebugRenderer::AddPolygon(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, const Color& color)
{
	AddTriangle(a, b, c, color);
	AddTriangle(c, d, a, color);
}

void DebugRenderer::AddBoundingBox(const BoundingBox& boundingBox, const Color& color, const bool solid /*= false*/)
{
	Vector3 min(boundingBox.Center.x - boundingBox.Extents.x, boundingBox.Center.y - boundingBox.Extents.y, boundingBox.Center.z - boundingBox.Extents.z);
	Vector3 max(boundingBox.Center.x + boundingBox.Extents.x, boundingBox.Center.y + boundingBox.Extents.y, boundingBox.Center.z + boundingBox.Extents.z);

	Vector3 v1(max.x, min.y, min.z);
	Vector3 v2(max.x, max.y, min.z);
	Vector3 v3(min.x, max.y, min.z);
	Vector3 v4(min.x, min.y, max.z);
	Vector3 v5(max.x, min.y, max.z);
	Vector3 v6(min.x, max.y, max.z);

	if (!solid)
	{
		AddLine(min, v1, color);
		AddLine(v1, v2, color);
		AddLine(v2, v3, color);
		AddLine(v3, min, color);
		AddLine(v4, v5, color);
		AddLine(v5, max, color);
		AddLine(max, v6, color);
		AddLine(v6, v4, color);
		AddLine(min, v4, color);
		AddLine(v1, v5, color);
		AddLine(v2, max, color);
		AddLine(v3, v6, color);
	}
	else
	{
		AddPolygon(min, v1, v2, v3, color);
		AddPolygon(v4, v5, max, v6, color);
		AddPolygon(min, v4, v6, v3, color);
		AddPolygon(v1, v5, max, v2, color);
		AddPolygon(v3, v2, max, v6, color);
		AddPolygon(min, v1, v5, v4, color);
	}
}

void DebugRenderer::AddBoundingBox(const BoundingBox& boundingBox, const Matrix& transform, const Color& color, const bool solid /*= false*/)
{
	const Vector3 min(boundingBox.Center.x - boundingBox.Extents.x, boundingBox.Center.y - boundingBox.Extents.y, boundingBox.Center.z - boundingBox.Extents.z);
	const Vector3 max(boundingBox.Center.x + boundingBox.Extents.x, boundingBox.Center.y + boundingBox.Extents.y, boundingBox.Center.z + boundingBox.Extents.z);

	Vector3 v0(Vector3::Transform(min, transform));
	Vector3 v1(Vector3::Transform(Vector3(max.x, min.y, min.z), transform));
	Vector3 v2(Vector3::Transform(Vector3(max.x, max.y, min.z), transform));
	Vector3 v3(Vector3::Transform(Vector3(min.x, max.y, min.z), transform));
	Vector3 v4(Vector3::Transform(Vector3(min.x, min.y, max.z), transform));
	Vector3 v5(Vector3::Transform(Vector3(max.x, min.y, max.z), transform));
	Vector3 v6(Vector3::Transform(Vector3(min.x, max.y, max.z), transform));
	Vector3 v7(Vector3::Transform(max, transform));

	if (!solid)
    {
        AddLine(v0, v1, color);
        AddLine(v1, v2, color);
        AddLine(v2, v3, color);
        AddLine(v3, v0, color);
        AddLine(v4, v5, color);
        AddLine(v5, v7, color);
        AddLine(v7, v6, color);
        AddLine(v6, v4, color);
        AddLine(v0, v4, color);
        AddLine(v1, v5, color);
        AddLine(v2, v7, color);
        AddLine(v3, v6, color);
    }
    else
    {
        AddPolygon(v0, v1, v2, v3, color);
        AddPolygon(v4, v5, v7, v6, color);
        AddPolygon(v0, v4, v6, v3, color);
        AddPolygon(v1, v5, v7, v2, color);
        AddPolygon(v3, v2, v7, v6, color);
        AddPolygon(v0, v1, v5, v4, color);
    }
}

void DebugRenderer::AddSphere(const Vector3& position, const float radius, const int slices, const int stacks, const Color& color, const bool solid)
{
	DebugSphere sphere(position, radius);

	float jStep = XM_PI / slices;
	float iStep = XM_PI / stacks;

	if (!solid)
	{
		for (float j = 0; j < XM_PI; j += jStep)
		{
			for (float i = 0; i < XM_2PI; i += iStep)
			{
				Vector3 p1 = sphere.GetPoint(i, j);
				Vector3 p2 = sphere.GetPoint(i + iStep, j);
				Vector3 p3 = sphere.GetPoint(i, j + jStep);
				Vector3 p4 = sphere.GetPoint(i + iStep, j + jStep);

				AddLine(p1, p2, color);
				AddLine(p3, p4, color);
				AddLine(p1, p3, color);
				AddLine(p2, p4, color);
			}
		}
	}
	else
	{
		for (float j = 0; j < XM_PI; j += jStep)
		{
			for (float i = 0; i < XM_2PI; i += iStep)
			{
				Vector3 p1 = sphere.GetPoint(i, j);
				Vector3 p2 = sphere.GetPoint(i + iStep, j);
				Vector3 p3 = sphere.GetPoint(i, j + jStep);
				Vector3 p4 = sphere.GetPoint(i + iStep, j + jStep);

				AddPolygon(p2, p1, p3, p4, (Color)Colors::Blue);
			}
		}
	}
}

void DebugRenderer::AddFrustrum(const BoundingFrustum& frustrum, const Color& color)
{
	vector<XMFLOAT3> corners(frustrum.CORNER_COUNT);
	frustrum.GetCorners(corners.data());

	AddLine(corners[0], corners[1], color);
	AddLine(corners[1], corners[2], color);
	AddLine(corners[2], corners[3], color);
	AddLine(corners[3], corners[0], color);
	AddLine(corners[4], corners[5], color);
	AddLine(corners[5], corners[6], color);
	AddLine(corners[6], corners[7], color);
	AddLine(corners[7], corners[4], color);
	AddLine(corners[0], corners[4], color);
	AddLine(corners[1], corners[5], color);
	AddLine(corners[2], corners[6], color);
	AddLine(corners[3], corners[7], color);
}

void DebugRenderer::AddAxisSystem(const Matrix& transform, const float lineLength)
{
	Vector3 origin(Vector3::Transform(Vector3(), transform));
	Vector3 x(Vector3::Transform(Vector3(lineLength, 0, 0), transform));
	Vector3 y(Vector3::Transform(Vector3(0, lineLength, 0), transform));
	Vector3 z(Vector3::Transform(Vector3(0, 0, lineLength), transform));

	AddLine(origin, x, Color(1, 0, 0, 1));
	AddLine(origin, y, Color(0, 1, 0, 1));
	AddLine(origin, z, Color(0, 0, 1, 1));
}

void DebugRenderer::AddPhysicsScene(PhysicsScene* pScene)
{
	if (pScene == nullptr)
		return;

	const PxRenderBuffer& pBuffer = pScene->GetScene()->getRenderBuffer();
	const PxDebugLine* pLines = pBuffer.getLines();

	HexColorConverter converter;
	for (unsigned int i = 0; i < pBuffer.getNbLines(); ++i)
	{
		AddLine(
			*reinterpret_cast<const Vector3*>(&pLines[i].pos0),
			*reinterpret_cast<const Vector3*>(&pLines[i].pos1),
			converter(pLines[i].color0),
			converter(pLines[i].color1)
		);
	}

	const PxDebugTriangle* pTriangles = pBuffer.getTriangles();
	for (unsigned int i = 0; i < pBuffer.getNbTriangles(); ++i)
	{
		AddTriangle(
			*reinterpret_cast<const Vector3*>(&pTriangles[i].pos0),
			*reinterpret_cast<const Vector3*>(&pTriangles[i].pos1),
			*reinterpret_cast<const Vector3*>(&pTriangles[i].pos2),
			converter(pTriangles[i].color0),
			converter(pTriangles[i].color1),
			converter(pTriangles[i].color2)
		);
	}
}

void DebugRenderer::AddMesh(Mesh* pMesh, const Vector3& position, const Color& color, const bool solid)
{
	if (pMesh == nullptr)
		return;
	for (int i = 0; i < pMesh->GetGeometryCount(); ++i)
	{
		Geometry* pGeometry = pMesh->GetGeometry(i);
		if (pGeometry == nullptr)
			continue;
		const Geometry::VertexData& vertexData = pGeometry->GetVertexData("POSITION");
		if (pGeometry->HasData("INDEX"))
		{
			const Geometry::VertexData& indexData = pGeometry->GetVertexData("INDEX");
			for (int index = 0; index < indexData.Count; index += 3)
			{
				const unsigned int indexA = static_cast<unsigned int*>(indexData.pData)[index];
				const unsigned int indexB = static_cast<unsigned int*>(indexData.pData)[index + 1];
				const unsigned int indexC = static_cast<unsigned int*>(indexData.pData)[index + 2];

				Vector3 vertexA = static_cast<Vector3*>(vertexData.pData)[indexA] + position;
				Vector3 vertexB = static_cast<Vector3*>(vertexData.pData)[indexB] + position;
				Vector3 vertexC = static_cast<Vector3*>(vertexData.pData)[indexC] + position;

				AddTriangle(vertexA, vertexB, vertexC, color, solid);
			}
		}
		else
		{
			for (int index = 0; index < vertexData.Count; index += 3)
			{
				Vector3 vertexA = static_cast<Vector3*>(vertexData.pData)[index] + position;
				Vector3 vertexB = static_cast<Vector3*>(vertexData.pData)[index + 1] + position;
				Vector3 vertexC = static_cast<Vector3*>(vertexData.pData)[index + 2] + position;

				AddTriangle(vertexA, vertexB, vertexC, color, solid);
			}
		}
	}
}
