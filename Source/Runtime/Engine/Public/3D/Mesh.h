#pragma once

namespace CK
{
namespace DDD
{

class Mesh
{
public:
	Mesh() {};
	~Mesh() {};

public:
	std::vector<Vector2> _UVs;
	std::vector<Vector3> _Vertices;
	std::vector<int> _Indices;

private:
};

}
}