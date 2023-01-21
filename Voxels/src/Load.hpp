#pragma once
#include "Octree.hpp"

namespace Util {
Octree LoadOBJ(const char *filepath, int size, int depth);
}