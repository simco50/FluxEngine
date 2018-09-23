#include "FluxEngine.h"
#include "Geometry.h"
#include "Rendering/Core/Graphics.h"

Geometry::Geometry()
{

}

Geometry::~Geometry()
{
	for (auto& vertexData : m_VertexData)
	{
		delete vertexData.second.pData;
	}
}

void Geometry::Draw(Graphics* pGraphics) const
{
	pGraphics->SetIndexBuffer(m_pIndexBuffer);
	pGraphics->SetVertexBuffer(m_pVertexBuffer);
	if (m_pIndexBuffer)
		pGraphics->DrawIndexed(m_PrimitiveType, m_IndexCount, 0);
	else
		pGraphics->Draw(m_PrimitiveType, 0, m_VertexCount);
}

Geometry::VertexData& Geometry::GetVertexData(const std::string& semantic)
{
	auto it = m_VertexData.find(semantic);
	if (it == m_VertexData.end())
		FLUX_LOG(Error, "MeshFilter::GetVertexData() > VertexData with semantic '%s' not found.", semantic.c_str());
	return it->second;
}

Geometry::VertexData& Geometry::GetVertexDataUnsafe(const std::string& semantic)
{
	return m_VertexData[semantic];
}

bool Geometry::HasData(const std::string& semantic) const
{
	auto it = m_VertexData.find(semantic);
	return !(it == m_VertexData.end());
}

int Geometry::GetDataCount(const std::string& semantic)
{
	auto pIt = m_VertexData.find(semantic);
	if (pIt == m_VertexData.end())
		return 0;
	return pIt->second.Count;
}

void Geometry::SetDrawRange(PrimitiveType type, int indexCount, int vertexCount)
{
	m_PrimitiveType = type;
	m_IndexCount = indexCount;
	m_VertexCount = vertexCount;
}
