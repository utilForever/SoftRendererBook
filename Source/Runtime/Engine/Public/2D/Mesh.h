#pragma once

namespace CK
{
namespace DD
{

class Mesh
{
public:
	Mesh() {};
	~Mesh() {};

	std::vector<Vector2> _Vertices;
	std::vector<int> _Indices;
};

}
}
