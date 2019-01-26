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
		delete (char*)vertexData.second.pData;
	}
}

void Geometry::Draw(Graphics* pGraphics) const
{
	if (m_pVertexBuffer)
	{
		pGraphics->SetIndexBuffer(m_pIndexBuffer);
		pGraphics->SetVertexBuffer(m_pVertexBuffer);
		if (m_pIndexBuffer)
		{
			pGraphics->DrawIndexed(m_PrimitiveType, m_IndexCount, 0);
		}
		else
		{
			pGraphics->Draw(m_PrimitiveType, 0, m_VertexCount);
		}
	}
}

Geometry::VertexData& Geometry::GetVertexData(const std::string& semantic, const int slot)
{
	std::string lookupString = GetLookupStringFromSemantic(semantic, slot);
	auto it = m_VertexData.find(lookupString);
	if (it == m_VertexData.end())
	{
		FLUX_LOG(Error, "MeshFilter::GetVertexData() > VertexData with semantic '%s' not found.", lookupString.c_str());
	}
	return it->second;
}

Geometry::VertexData& Geometry::GetVertexDataUnsafe(const std::string& semantic, const int slot)
{
	std::string lookupString = GetLookupStringFromSemantic(semantic, slot);
	return m_VertexData[lookupString];
}

bool Geometry::HasData(const std::string& semantic, const int slot) const
{
	std::string lookupString = GetLookupStringFromSemantic(semantic, slot);
	auto it = m_VertexData.find(lookupString);
	return !(it == m_VertexData.end());
}

int Geometry::GetDataCount(const std::string& semantic, const int slot) const
{
	std::string lookupString = GetLookupStringFromSemantic(semantic, slot);
	auto pIt = m_VertexData.find(lookupString);
	if (pIt == m_VertexData.end())
	{
		return 0;
	}
	return pIt->second.Count;
}

void Geometry::SetDrawRange(PrimitiveType type, int indexCount, int vertexCount)
{
	m_PrimitiveType = type;
	m_IndexCount = indexCount;
	m_VertexCount = vertexCount;
}

int Geometry::GetSize() const
{
	int size = 0;
	for (const auto& p : m_VertexData)
	{
		size += p.second.ByteSize();
	}
	return size;
}

std::string Geometry::GetLookupStringFromSemantic(const std::string& semantic, const int slot)
{
	return Printf("%s%d", semantic.c_str(), slot);
}
