#include "Load.hpp"
#include "DataType.hpp"

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
                for (int i = 0; i < 3; i++) {
                    int index = line.find('/') < line.find(' ') ? line.find('/') : line.find(' ');
                    f[i] = std::stoi(line.substr(0, index));
                    line.erase(0, index + 1);
                }
                f[i] = f[0];
            }
            faces.push_back(f);
        }
    }

    Octree ot(size, depth);
    float min = std::min(std::min(modelMin.x, modelMin.y), modelMin.z);
    float max = std::max(std::max(modelMax.x, modelMax.y), modelMax.z);

    std::cout << "Vertex Count: " << positions.size() << '\n';

    for (auto face : faces) {
        // Triangle t;
        for (int i = 0; i < 3; i++) {
            Vector3f p = {
                ((positions[face[i] - 1].x + std::abs(min)) / (std::abs(min) + max)) * size,
                ((positions[face[i] - 1].y + std::abs(min)) / (std::abs(min) + max)) * size,
                ((positions[face[i] - 1].z + std::abs(min)) / (std::abs(min) + max)) * size,
            };
            // t[i] = p;
            ot.Insert(p, {255, 255, 255, 255});
        }
    }

    return ot;
}

}  // namespace Util