#include "Octree.hpp"

#include <cmath>
#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <bitset>

static int childCount = 0;
static int voxelCount = 0;

Octree::Octree(int size, int maxDepth) : m_Size(size), m_MaxDepth(maxDepth), m_Root(nullptr) {}

void Octree::Insert(Vector3f point, Color color) {
    Insert(&m_Root, point, color, Vector3i{0}, 0);
}

void Octree::CreateBuffer() {
    int i = 0;
    CreateBuffer(m_Root, i);
    std::cout << "Total voxels: " << voxelCount << '\n';
}

void Octree::Insert(Node **node, Vector3f point, Color color, Vector3i position, int depth) {
    // We only create nodes that have values, hence the sparse name
    if (*node == nullptr) {
        *node = new Node;
    }

    if (depth == m_MaxDepth) {
        (*node)->IsLeaf = true;
        (*node)->data.color = color;
        return;
    }

    /*
        We get the size of the current voxel, which we use to find the
        lower boundary of the volume by multiplying it with all the previous positions.
        Once that is aquired we take the point we are
        intersted in and minus it by that boundary, which is then divided by the size
        of the current voxel. This gives us a number between 0-1 which we round to find out
        which child this point belongs to.
    */
    (*node)->data.color = color;
    float size = std::exp2(-depth) * m_Size;
    Vector3i childPos = {
        (int)std::round((point.x - ((float)position.x * size)) / size),
        (int)std::round((point.y - ((float)position.y * size)) / size),
        (int)std::round((point.z - ((float)position.z * size)) / size),
    };
    // We get the child index by or-ing together all the axis
    int childIndex = (childPos.x << 0) | (childPos.y << 1) | (childPos.z << 2);

    // To keep track of where we are in the tree we keep track of all the choices we have made so far.
    position = {
        (position.x << 1) | childPos.x,
        (position.y << 1) | childPos.y,
        (position.z << 1) | childPos.z,
    };

    // This continues until we are at our max resolution
    Insert(&(*node)->children[childIndex], point, color, position, ++depth);
}

unsigned int Octree::CreateDescriptor(Node *node, int &index, int pIndex) {
    unsigned int childDesc = 0;
    int ValidChildCount = 0;
    for (int i = 0; i < 8; i++) {
        if (Node *child = node->children[i]) {
            childDesc |= 1 << i;
            if (child->IsLeaf) {
                childDesc |= 1 << (i + 8);
                voxelCount++;
            } else {
                if (!ValidChildCount) {
                    if ((index - pIndex) >= std::exp2(15))
                        childDesc |= 1 << 16;

                    childDesc |= (index - pIndex) << 17;
                    // std::cout << "Overflowing: Index -> " << (index - pIndex) << '\n';
                }
                ValidChildCount++;
            }
        }
    }

    childCount = ValidChildCount;
    index += ValidChildCount;
    return childDesc;
}

static int GetSetBitCount(int n) {
    if (n == 0)
        return 0;
    else
        return (n & 1) + GetSetBitCount(n >> 1);
}

void Octree::CreateBuffer(Node *node, int &index) {
    if (node == m_Root)
        m_Buffer.push_back(CreateDescriptor(node, ++index, 0));

    int pIndex = index - childCount;
    m_Buffer.resize(index);
    for (int i = 0; i < 8; i++) {
        if (Node *child = node->children[i]) {
            if (!child->IsLeaf) {
                m_Buffer.at(pIndex) = CreateDescriptor(child, index, pIndex);
                pIndex++;
                CreateBuffer(child, index);
            }
        }
    }
}
