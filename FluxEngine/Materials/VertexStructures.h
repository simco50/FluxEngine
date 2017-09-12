#pragma once

struct VertexPosNormCol
{
	VertexPosNormCol(){}
	VertexPosNormCol(const XMFLOAT3& position, const XMFLOAT3& normal, const XMFLOAT4& color):
		Position(position), Normal(normal), Color(color)
	{}
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT4 Color;
};

struct VertexPosNormTex
{
	VertexPosNormTex() {}
	VertexPosNormTex(const XMFLOAT3& position, const XMFLOAT3& normal, const XMFLOAT2& texCoord) :
		Position(position), Normal(normal), TexCoord(texCoord)
	{}
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
};

struct VertexPosNorm
{
	VertexPosNorm() {}
	VertexPosNorm(const XMFLOAT3& position, const XMFLOAT3& normal) :
		Position(position), Normal(normal)
	{}
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
};
struct VertexPosTex
{
	VertexPosTex(){}
	VertexPosTex(const XMFLOAT3& p, const XMFLOAT2& t) :
		Position(p), TexCoord(t) {}
	XMFLOAT3 Position;
	XMFLOAT2 TexCoord;
};