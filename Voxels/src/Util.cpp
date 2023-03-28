#include "Util.hpp"
#include "AABBTriangle.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <iostream>

namespace Util {

Octree LoadOBJ(const char *filepath, int size, int depth) {
    std::ifstream fp(filepath);
    std::string line;
    if (!fp) {
        std::printf("ERROR::LOADER_CPP::LOADOBJ::COUDL_NOT_OPEN_FILE -> %s\n", filepath);
    }
    std::vector<glm::vec3> positions;
    std::vector<glm::ivec3> faces;

    glm::vec3 modelMax{0};
    glm::vec3 modelMin{100000.f};

    while (std::getline(fp, line)) {
        if (line.rfind("v ") == 0) {
            line.erase(0, line.find(' ') + 1);
            glm::vec3 pos;
            for (int i = 0; i < 3; i++) {
                int index = line.find(' ');
                pos[i] = std::stof(line.substr(0, index));
                if (pos[i] > modelMax[i]) modelMax[i] = pos[i];
                if (pos[i] < modelMin[i]) modelMin[i] = pos[i];
                line.erase(0, index + 1);
            }
            positions.push_back(pos);
        }
        if (line.rfind("f ") == 0) {
            line.erase(0, line.find(' ') + 1);
            glm::ivec3 f;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    int index = line.find('/') < line.find(' ') ? line.find('/') : line.find(' ');
                    if (j == 0)
                        f[i] = std::stoi(line.substr(0, index));
                    else
                        std::stoi(line.substr(0, index));
                    line.erase(0, index + 1);
                }
            }
            faces.push_back(f);
        }
    }

    Octree ot(size, depth);
    float min = std::min(std::min(modelMin.x, modelMin.y), modelMin.z);
    float max = std::max(std::max(modelMax.x, modelMax.y), modelMax.z);

    std::cout << "Loading model...\n";
    int index = 0;

    for (auto face : faces) {
        if (index % (faces.size() / 100) == 0) std::cout << (int)((float)index / (float)faces.size() * 100.) << "%\n";
        Vector3f triangle[3];
        for (int i = 0; i < 3; i++) {
            triangle[i] = {
                ((positions[face[i] - 1].x + std::abs(min)) / (std::abs(min) + max)) * size,
                ((positions[face[i] - 1].y + std::abs(min)) / (std::abs(min) + max)) * size,
                ((positions[face[i] - 1].z + std::abs(min)) / (std::abs(min) + max)) * size,
            };
            // ot.Insert(triangle[i], {255, 255, 255, 255});
        }
        for (auto p : Voxelize(triangle, size, depth)) {
            ot.Insert(p, {255, 255, 255, 255});
        }

        index++;
    }

    std::cout << "100%\n";
    std::cout << "Model loaded!\n";

    return ot;
}

std::vector<Vector3f> Voxelize(Vector3f *triangle, int size, int depth) {
    Vector3f max = {0, 0, 0}, min{1000, 1000, 1000};

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (triangle[i][j] < min[j]) min[j] = triangle[i][j];
            if (triangle[i][j] > max[j]) max[j] = triangle[i][j];
        }
    }

    float s = std::exp2(-depth) * size;

    max = {std::ceil(max.x / s) * s, std::ceil(max.y / s) * s, std::ceil(max.z / s) * s};
    min = {std::floor(min.x / s) * s, std::floor(min.y / s) * s, std::floor(min.z / s) * s};
    std::vector<Vector3f> overlapPoints;

    for (int z = (int)(min.z / s); z <= (int)(max.z / s); z++) {
        for (int y = (int)(min.y / s); y <= (int)(max.y / s); y++) {
            for (int x = (int)(min.x / s); x <= (int)(max.x / s); x++) {
                Vector3f halfWidth = {s / 2.f, s / 2.f, s / 2.f};
                Vector3f center = Vector3f{x * s, y * s, z * s} +
                                  Vector3f{
                                      x == (int)(max.x / s) ? -halfWidth.x : halfWidth.x,
                                      y == (int)(max.y / s) ? -halfWidth.y : halfWidth.y,
                                      z == (int)(max.z / s) ? -halfWidth.z : halfWidth.z,
                                  };
                if (triBoxOverlap(center, halfWidth, triangle)) {
                    overlapPoints.push_back(center);
                }
            }
        }
    }

    return overlapPoints;
}

}  // namespace Util