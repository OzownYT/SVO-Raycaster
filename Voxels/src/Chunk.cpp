#include "Chunk.hpp"

Chunk::Chunk(glm::ivec3 size) {
    int elemCount = size.x * size.y * size.z;
    m_Size = size;
    m_Colors.resize(elemCount);
    for (int i = 0; i < elemCount; ++i) {
        m_Colors[i] = glm::vec4(0, 0, 0, 0);
    }
}

Chunk::~Chunk() {}

void Chunk::MakeSphere() {
    float radius = m_Size.x / 2.0f - 0.5f;
    glm::vec3 color = 1.0f / (glm::vec3(m_Size) - glm::vec3(1, 1, 1));
    glm::vec3 center = glm::vec3(m_Size) / 2.0f;

    for (int z = 0; z < m_Size.z; ++z) {
        for (int y = 0; y < m_Size.y; ++y) {
            for (int x = 0; x < m_Size.x; ++x) {
                glm::ivec3 idx(x, y, z);

                glm::vec3 delta = (glm::vec3(idx) + glm::vec3(0.5f, 0.5f, 0.5f)) - center;

                if ((glm::dot(delta, delta) - 0.1f) <= radius * radius) {
                    // Inside sphere
                    At(idx) = glm::vec4(color * glm::vec3(idx), 1.0f);
                } else {
                    // Outside sphere
                    At(idx) = glm::vec4(0, 0, 0, 0);
                }
            }
        }
    }
}

glm::vec4& Chunk::At(glm::ivec3 idx) {
    return m_Colors[idx.z * (m_Size.x * m_Size.y) + idx.y * m_Size.x + idx.x];
}