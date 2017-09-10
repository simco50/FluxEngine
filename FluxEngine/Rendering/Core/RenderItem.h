#pragma once
class Material;
class MeshFilter;
class RenderTarget;

struct RenderItem
{
	RenderItem()
	{}

	D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	ID3D11Buffer* IndexBuffer = nullptr;
	vector<ID3D11Buffer*> VertexBuffers;
	int VertexCount = -1;
	int IndexCount = -1;

	XMFLOAT4X4 WorldMatrix = {};
	Material* Material = nullptr;
	RenderTarget* RenderTarget = nullptr;
};

