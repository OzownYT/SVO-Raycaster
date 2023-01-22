#pragma once
#include "DataType.hpp"

#include <glm/glm.hpp>
#include <vector>

struct Node {
    bool IsLeaf;
    Node *children[8];
    VoxelInfo data;

    Node() : IsLeaf(false) {
        for (int i = 0; i < 8; i++) {
            children[i] = nullptr;
        }
    }
};

class Octree {
   public:
    Octree(int size, int maxDepth);

    void Insert(Vector3f point, Color color);
    void CreateBuffer();
    unsigned int *GetBuffer() {
        return m_Buffer.data();
    }
    std::vector<unsigned int> GetVector() {
        return m_Buffer;
    }

    unsigned int GetSize() const { return m_Buffer.size() * sizeof(unsigned int); }
    unsigned int GetLength() const { return m_Buffer.size(); }

   private:
    void Insert(Node **node, Vector3f point, Color color, Vector3i position, int depth);
    unsigned int CreateDescriptor(Node *node, int &index, int pIndex);
    void CreateBuffer(Node *node, int &index);

   private:
    Node *m_Root;
    int m_Size, m_MaxDepth;
    std::vector<unsigned int> m_Buffer;
    std::vector<uint8_t> colors;
};