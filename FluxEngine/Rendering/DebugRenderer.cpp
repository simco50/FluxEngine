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
#include "Light.h"
#include "IO/MemoryStream.h"
#include "Scenegraph\SceneNode.h"

DebugRenderer::DebugRenderer(Context* pContext) :
	Subsystem(pContext)
{
	m_pGraphics = pContext->GetSubsystem<Graphics>();

	m_ElementDesc =
	{
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::POSITION, 0, false),
		VertexElement(VertexElementType::FLOAT4, VertexElementSemantic::COLOR, 0, false),
	};
	m_pVertexShader = m_pGraphics->GetShader("Shaders/DebugRenderer", ShaderType::VertexShader);
	m_pPixelShader = m_pGraphics->GetShader("Shaders/DebugRenderer", ShaderType::PixelShader);
	m_pVertexBuffer = std::make_unique<VertexBuffer>(m_pGraphics);
}

DebugRenderer::~DebugRenderer()
{

}

void DebugRenderer::Render()
{
	int totalPrimitives = m_LinePrimitives + m_TrianglePrimitives;

	if (totalPrimitives == 0 || m_pCamera == nullptr)
		return;

	AUTOPROFILE(DebugRenderer_Render);

	m_pGraphics->SetShader(ShaderType::VertexShader, m_pVertexShader);
	m_pGraphics->SetShader(ShaderType::PixelShader, m_pPixelShader);
	m_pGraphics->SetShader(ShaderType::ComputeShader, nullptr);
	m_pGraphics->SetShader(ShaderType::GeometryShader, nullptr);

	if (totalPrimitives > (int)m_pVertexBuffer->GetVertexCount())
	{
		m_pVertexBuffer->Create(totalPrimitives + 100, m_ElementDesc, true);
	}

	void* pData = m_pVertexBuffer->Map(true);
	MemoryStream memStream(pData, m_pVertexBuffer->GetSize());
	for (const DebugLine& line : m_Lines)
	{
		memStream.Write(&line.Start, sizeof(Vector3));
		memStream.Write(&line.ColorStart, sizeof(Color));
		memStream.Write(&line.End, sizeof(Vector3));
		memStream.Write(&line.ColorEnd, sizeof(Color));
	}
	for (const DebugTriangle& triangle : m_Triangles)
	{
		memStream.Write(&triangle.A, sizeof(Vector3));
		memStream.Write(&triangle.ColorA, sizeof(Color));
		memStream.Write(&triangle.B, sizeof(Vector3));
		memStream.Write(&triangle.ColorB, sizeof(Color));
		memStream.Write(&triangle.C, sizeof(Vector3));
		memStream.Write(&triangle.ColorC, sizeof(Color));
	}

	m_pVertexBuffer->Unmap();

	m_pGraphics->SetVertexBuffer(m_pVertexBuffer.get());
	m_pGraphics->SetIndexBuffer(nullptr);

	m_pGraphics->GetDepthStencilState()->SetDepthEnabled(true);
	m_pGraphics->GetDepthStencilState()->SetDepthTest(CompareMode::LESSEQUAL);

	m_pGraphics->GetBlendState()->SetColorWrite(ColorWrite::ALL);
	m_pGraphics->GetBlendState()->SetBlendMode(BlendMode::REPLACE, false);

	m_pGraphics->GetRasterizerState()->SetCullMode(CullMode::BACK);
	m_pGraphics->GetRasterizerState()->SetFillMode(FillMode::SOLID);

	Matrix projectionMatrix = m_pCamera->GetViewProjection();
	m_pGraphics->SetShaderParameter(ShaderConstant::cViewProj, &projectionMatrix);

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

	float jStep = Math::PI / slices;
	float iStep = Math::PI / stacks;

	if (!solid)
	{
		for (float j = 0; j < Math::PI; j += jStep)
		{
			for (float i = 0; i < Math::PI * 2; i += iStep)
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
		for (float j = 0; j < Math::PI; j += jStep)
		{
			for (float i = 0; i < Math::PI * 2; i += iStep)
			{
				Vector3 p1 = sphere.GetPoint(i, j);
				Vector3 p2 = sphere.GetPoint(i + iStep, j);
				Vector3 p3 = sphere.GetPoint(i, j + jStep);
				Vector3 p4 = sphere.GetPoint(i + iStep, j + jStep);

				AddPolygon(p2, p1, p3, p4, Color(0, 0, 1, 1));
			}
		}
	}
}

void DebugRenderer::AddFrustrum(const BoundingFrustum& frustrum, const Color& color)
{
	std::vector<Vector3> corners(BoundingFrustum::CORNER_COUNT);
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
	Matrix newMatrix = Matrix::CreateScale(Math::ScaleFromMatrix(transform));
	newMatrix.Invert(newMatrix);
	newMatrix *= Matrix::CreateScale(Vector3::Distance(m_pCamera->GetNode()->GetWorldPosition(), transform.Translation()) / 5.0f);
	newMatrix *= transform;
	Vector3 origin(Vector3::Transform(Vector3(), transform));
	Vector3 x(Vector3::Transform(Vector3(lineLength, 0, 0), newMatrix));
	Vector3 y(Vector3::Transform(Vector3(0, lineLength, 0), newMatrix));
	Vector3 z(Vector3::Transform(Vector3(0, 0, lineLength), newMatrix));

	AddLine(origin, x, Color(1, 0, 0, 1));
	AddLine(origin, y, Color(0, 1, 0, 1));
	AddLine(origin, z, Color(0, 0, 1, 1));
}

void DebugRenderer::AddPhysicsScene(PhysicsScene* pScene)
{
	using namespace physx;

	if (pScene == nullptr)
		return;

	const PxRenderBuffer& pBuffer = pScene->GetScene()->getRenderBuffer();
	const PxDebugLine* pLines = pBuffer.getLines();

	Math::HexColorConverter converter;
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
	AddMesh(pMesh, Matrix::CreateTranslation(position), color, solid);
}

void DebugRenderer::AddMesh(Mesh* pMesh, const Matrix& worldMatrix, const Color& color, const bool solid /*= false*/)
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

				Vector3 vertexA = Vector3::Transform(static_cast<Vector3*>(vertexData.pData)[indexA], worldMatrix);
				Vector3 vertexB = Vector3::Transform(static_cast<Vector3*>(vertexData.pData)[indexB], worldMatrix);
				Vector3 vertexC = Vector3::Transform(static_cast<Vector3*>(vertexData.pData)[indexC], worldMatrix);

				AddTriangle(vertexA, vertexB, vertexC, color, solid);
			}
		}
		else
		{
			for (int index = 0; index < vertexData.Count; index += 3)
			{
				Vector3 vertexA = Vector3::Transform(static_cast<Vector3*>(vertexData.pData)[index], worldMatrix);
				Vector3 vertexB = Vector3::Transform(static_cast<Vector3*>(vertexData.pData)[index + 1], worldMatrix);
				Vector3 vertexC = Vector3::Transform(static_cast<Vector3*>(vertexData.pData)[index + 2], worldMatrix);

				AddTriangle(vertexA, vertexB, vertexC, color, solid);
			}
		}
	}
}

void DebugRenderer::AddSkeleton(const Skeleton& skeleton, const Color& color)
{
	AddBoneRecursive(skeleton, skeleton.GetRootBoneIndex(), color);
}

void DebugRenderer::AddBone(const Matrix& matrix, const float length, const Color& color)
{
	float boneSize = 2;
	Vector3 start = Vector3::Transform(Vector3(0, 0, 0), matrix);
	Vector3 a = Vector3::Transform(Vector3(-boneSize, boneSize, boneSize), matrix);
	Vector3 b = Vector3::Transform(Vector3(boneSize, boneSize, boneSize), matrix);
	Vector3 c = Vector3::Transform(Vector3(boneSize, -boneSize, boneSize), matrix);
	Vector3 d = Vector3::Transform(Vector3(-boneSize, -boneSize, boneSize), matrix);
	Vector3 tip = Vector3::Transform(Vector3(0, 0, -boneSize * length), matrix);

	AddTriangle(start, d, c, color, color, color, false);
	AddTriangle(start, a, d, color, color, color, false);
	AddTriangle(start, b, a, color, color, color, false);
	AddTriangle(start, c, b, color, color, color, false);
	AddTriangle(d, tip, c, color, color, color, false);
	AddTriangle(a, tip, d, color, color, color, false);
	AddTriangle(b, tip, a, color, color, color, false);
	AddTriangle(c, tip, b, color, color, color, false);
}

void DebugRenderer::AddLight(const Light* pLight)
{
	const Light::Data* pData = pLight->GetData();
	switch (pData->Type)
	{
	case Light::Type::Directional:
		AddWireCylinder(pData->Position, pData->Direction, 200.0f, 50.0f, 10, Color(1.0f, 1.0f, 0.0f, 1.0f));
		break;
	case Light::Type::Point:
		AddSphere(pData->Position, pData->Range, 8, 8, Color(1.0f, 1.0f, 0.0f, 1.0f), false);
		break;
	case Light::Type::Spot:
		AddWireCone(pData->Position, pData->Direction, pData->Range, pData->SpotLightAngle, 10, Color(1.0f, 1.0f, 0.0f, 1.0f));
		break;
	default:
		break;
	}
}

void DebugRenderer::AddWireCylinder(const Vector3& position, const Vector3& direction, const float height, const float radius, const int segments, const Color& color)
{
	Vector3 d;
	direction.Normalize(d);

	DebugSphere sphere(position, radius);
	float t = Math::PI * 2 / (segments + 1);

	Matrix world = Matrix::CreateFromQuaternion(Math::LookRotation(d)) * Matrix::CreateTranslation(position - d * (height / 2));
	for (int i = 0; i < segments + 1; ++i)
	{
		Vector3 a = Vector3::Transform(sphere.GetLocalPoint(Math::PIDIV2, i * t), world);
		Vector3 b = Vector3::Transform(sphere.GetLocalPoint(Math::PIDIV2, (i + 1) * t), world);
		AddLine(a, b, color, color);
		AddLine(a + d * height, b + d * height, color, color);
		AddLine(a, a + d * height, color, color);
	}
}

void DebugRenderer::AddWireCone(const Vector3& position, const Vector3& direction, const float height, const float angle, const int segments, const Color& color)
{
	Vector3 d;
	direction.Normalize(d);

	float radius = tan(Math::ToRadians * angle) * height;
	DebugSphere sphere(position, radius);
	float t = Math::PI * 2 / (segments + 1);

	Matrix world = Matrix::CreateFromQuaternion(Math::LookRotation(d)) * Matrix::CreateTranslation(position);
	for (int i = 0; i < segments + 1; ++i)
	{
		Vector3 a = Vector3::Transform(sphere.GetLocalPoint(Math::PIDIV2, i * t), world) + direction * height;
		Vector3 b = Vector3::Transform(sphere.GetLocalPoint(Math::PIDIV2, (i + 1) * t), world) + direction * height;
		AddLine(a, b, color, color);
		AddLine(a, position, color, color);
	}
}

void DebugRenderer::AddBoneRecursive(const Skeleton & skeleton, const int boneIndex, const Color & color)
{
	const Bone* pBone = skeleton.GetBone(boneIndex);
	AddBone(pBone->pNode->GetWorldMatrix(), 5.0f, color);

	for (int i : pBone->Children)
	{
		const Bone* pChild = skeleton.GetBone(i);
		AddBoneRecursive(skeleton, (int)i, color);
		AddLine(pBone->pNode->GetWorldPosition(), pChild->pNode->GetWorldPosition(), color);
	}
}
