#pragma once
#include "Octree.hpp"
#include "DataType.hpp"

namespace Util {
Octree LoadOBJ(const char *filepath, int size, int depth);
std::vector<Vector3f> Voxelize(Vector3f *triangle, int size, int depth);

}  // namespace Util