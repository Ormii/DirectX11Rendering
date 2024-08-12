#pragma once
#include <SimpleMath.h>
#include <vector>

using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;

struct MeshVertex
{
	Vector3 pos;
	Vector3 normal;
	Vector2 texcoord;
};

struct MeshData
{
	std::vector<MeshVertex> vertices;
	std::vector<uint16> indices;
};

class MeshGenerator
{
public:
	static MeshData MakeSquare();
};

