#pragma once
#include <glm/glm.hpp>
#include <vector>

struct Voxel {
    glm::vec3 Color;
};

class Chunk {
   public:
    Chunk(glm::ivec3 size);
    ~Chunk();

    inline bool IsValid(glm::ivec3 idx) {
        return idx.x >= 0 && idx.x < m_Size.x &&
               idx.y >= 0 && idx.y < m_Size.y &&
               idx.z >= 0 && idx.z < m_Size.z;
    }

    inline bool IsSolid(glm::ivec3 idx) {
        return IsValid(idx) && At(idx).a > 0.1f;
    }

    glm::vec4 &At(glm::ivec3 idx);
    inline glm::ivec3 GetSize() const { return m_Size; }

    // Make the voxel set into a colored sphere
    void MakeSphere();

   private:
    glm::ivec3 m_Size;
    std::vector<glm::vec4> m_Colors;
};