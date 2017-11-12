#include "stdafx.h"
#include "Geometry.h"
#include "Rendering/Core/Graphics.h"

Geometry::Geometry()
{

}

Geometry::~Geometry()
{
	for (auto& vertexData : m_VertexData)
		SafeDeleteArray(vertexData.second.pData);
}

void Geometry::Draw(Graphics* pGraphics) const
{
	pGraphics->SetIndexBuffer(m_pIndexBuffer);
	pGraphics->SetVertexBuffer(m_pVertexBuffer);
	if (m_pIndexBuffer)
		pGraphics->DrawIndexed(PrimitiveType::TRIANGLELIST, m_IndexCount, 0);
	else
		pGraphics->Draw(PrimitiveType::TRIANGLELIST, 0, m_VertexCount);
}

Geometry::VertexData& Geometry::GetVertexData(const string& semantic)
{
	auto it = m_VertexData.find(semantic);
	if (it == m_VertexData.end())
		FLUX_LOG(ERROR, "MeshFilter::GetVertexData() > VertexData with semantic '%s' not found.", semantic.c_str());
	return it->second;
}

Geometry::VertexData& Geometry::GetVertexDataUnsafe(const string& semantic)
{
	return m_VertexData[semantic];
}

bool Geometry::HasData(const string& semantic) const
{
	auto it = m_VertexData.find(semantic);
	return !(it == m_VertexData.end());
}
