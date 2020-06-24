
#include "Precompiled.h"
using namespace CK::DD;

void Mesh::CalculateBounds()
{
	CircleBound = Circle(_Vertices.data(), _Vertices.size());
	RectangleBound = Rectangle(_Vertices.data(), _Vertices.size());
}