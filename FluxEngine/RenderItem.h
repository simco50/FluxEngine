#pragma once
class Material;
class MeshFilter;

struct RenderItem
{
	D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	
	MeshFilter* Mesh = nullptr;
	Material* Material = nullptr;
};

