#include "Window.hpp"
#include "Input.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Chunk.hpp"
#include "Octree.hpp"
#include "Load.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <stb_image.h>
#include <bitset>

glm::ivec3 GetMoveVector();
unsigned int CreateQuad();
unsigned int CreateTexture(unsigned int width, unsigned int height, bool imageTexture);
unsigned int VoxelsToTexture(Chunk &c);
unsigned int LoadCubemap(std::vector<std::string> faces);
void mouse_scroll(GLFWwindow *window, double xoff, double yoff);

int yOffset = 0;
const int SIZE = 100;
const int DEPTH = 8;

int main() {
    std::vector<std::string> faces{
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg"

    };

    Window wnd("Raymarching", 1280, 720);
    glfwSetScrollCallback(wnd.GetNativeWindow(), mouse_scroll);

    CmData cameraData = CmData();
    cameraData.Speed = 0.1f;
    Camera cm = Camera(glm::vec3(50.0f, 50.0f, SIZE * 2), glm::vec3(0.0f, 1.0f, 0.0f), cameraData);
    Shader quadShader("res/shaders/vShader.glsl", "res/shaders/fShader.glsl");
    Shader computeShader("res/shaders/cSVOTracer.glsl");
    cm.SetProjection(75.f, (float)wnd.GetWidth() / (float)wnd.GetHeight(), 0.1f, 100.f);

    unsigned int vao = CreateQuad();
    unsigned int tex = CreateTexture(wnd.GetWindowData().Width, wnd.GetWindowData().Height, true);
    unsigned int noiseTex = CreateTexture(2048, 2048, false);
    unsigned int skybox = LoadCubemap(faces);
    Chunk chunk({32, 32, 32});
    chunk.MakeSphere();
    unsigned int voxelTexture = VoxelsToTexture(chunk);

    glm::mat4 Model(0.0f);
    // Model = glm::translate(Model, glm::vec3(1.0f, 0.0f, 3.0f));
    glEnable(GL_DEPTH_TEST);

    Octree octree = Util::LoadOBJ("res/models/dragon.obj", SIZE, DEPTH);
    // Octree octree(SIZE, DEPTH);
    // std::vector<Vector3f> points;
    // points.push_back({80.f, 80.f, 80.f});
    // points.push_back({25.f, 25.f, 25.f});
    // points.push_back({15.f, 15.f, 15.f});

    // int i = 0;
    // for (auto p : points) {
    //     // Vector3f p{(float)(rand() % SIZE), (float)(rand() % SIZE), (float)(rand() % SIZE)};
    //     std::cout << "Point[" << (i++) << "] is (" << p.x << ", " << p.y << ", " << p.z << ")\n";
    //     octree.Insert(p, {255, 0, 0, 255});
    // }

    // std::vector<unsigned int> buffer = octree.GetVector();
    // for (int i = 0; i < buffer.size(); i++) {
    //     unsigned int val = buffer[i];

    //     std::bitset<8> valid(val);
    //     std::bitset<8> child(val >> 8);
    //     std::bitset<1> far(val >> 16);
    //     std::bitset<15> pointer(val >> 17);

    //     std::cout << pointer << " " << far << " " << child << " " << valid << '\n';
    // }
    unsigned int ssbo = 0;
    computeShader.Bind();
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    unsigned int bindIndex = 3;
    unsigned int blockIndex = glGetProgramResourceIndex(computeShader.GetID(), GL_SHADER_STORAGE_BLOCK, "octree");
    glShaderStorageBlockBinding(computeShader.GetID(), blockIndex, bindIndex);

    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindIndex, ssbo, 0, 3);
    glBufferData(GL_SHADER_STORAGE_BUFFER, octree.GetSize(), octree.GetBuffer(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindIndex, ssbo);

    while (wnd.IsRunning()) {
        if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
            wnd.SetRunning(false);

        if (Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
            cm.SetSpeed(3.f);
        else
            cm.SetSpeed(1.0f);
        wnd.Update();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cm.Update(GetMoveVector(), Input::GetMousePosition());
        if (yOffset) {
            cm.Zoom(yOffset);
            yOffset = 0;
        }

        computeShader.Bind();
        glBindTextureUnit(0, noiseTex);
        glBindTextureUnit(1, skybox);
        glBindTextureUnit(2, voxelTexture);
        computeShader.SetMat4fv("uCameraToWorld", glm::inverse(cm.GetView()));
        computeShader.SetMat4fv("uProjectionInverse", glm::inverse(cm.GetProjection()));
        computeShader.SetVec3fv("uLightPos", glm::vec3(0.0f, 5.0f, -6.0f));
        computeShader.Set1f("uTime", (float)glfwGetTime());
        computeShader.Set1i("uNoiseTexture", 0);
        computeShader.Set1i("uSkybox", 1);
        computeShader.Set1i("uVoxels", 2);
        computeShader.Set1i("uDepth", DEPTH);
        glDispatchCompute(wnd.GetWidth() / 32, (wnd.GetHeight() + 16) / 32, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        quadShader.Bind();
        quadShader.Set1i("uTexture", 0);
        glBindTextureUnit(0, tex);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

glm::ivec3 GetMoveVector() {
    glm::ivec3 mVec = glm::ivec3(0);
    if (Input::IsKeyPressed(GLFW_KEY_D)) mVec.x += 1;
    if (Input::IsKeyPressed(GLFW_KEY_A)) mVec.x -= 1;
    if (Input::IsKeyPressed(GLFW_KEY_SPACE)) mVec.y += 1;
    if (Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL)) mVec.y -= 1;
    if (Input::IsKeyPressed(GLFW_KEY_W)) mVec.z += 1;
    if (Input::IsKeyPressed(GLFW_KEY_S)) mVec.z -= 1;
    return mVec;
}

unsigned int CreateQuad() {
    // clang-format off
    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 

        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };
    // clang-format on

    unsigned int vao, vbo;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glCreateBuffers(1, &vbo);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);
    glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return vao;
}

unsigned int CreateTexture(unsigned int width, unsigned int height, bool imageTexture) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    if (imageTexture) {
        glBindImageTexture(0, textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    } else {
        int nWidth, nHeight, nrChannels;
        unsigned char *data = stbi_load("res/textures/noise.png", &nWidth, &nHeight, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else {
            std::cout << "Failed to load texture" << std::endl;
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

unsigned int LoadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(("res/textures/skybox/" + faces[i]).c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void mouse_scroll(GLFWwindow *window, double xoff, double yoff) {
    yOffset = yoff;
}

uint8_t RoundByteF(float f) {
    return (uint8_t)round(f * 255.0f);
}

unsigned int VoxelsToTexture(Chunk &c) {
    int totalSize = c.GetSize().x * c.GetSize().y * c.GetSize().z * 4;
    std::vector<uint8_t> colors(totalSize);

    for (int z = 0; z < c.GetSize().z; ++z) {
        for (int y = 0; y < c.GetSize().y; ++y) {
            for (int x = 0; x < c.GetSize().x; ++x) {
                glm::ivec3 idx(x, y, z);
                int colorIdx = z * c.GetSize().x * c.GetSize().y + y * c.GetSize().x + x;
                colorIdx *= 4;

                glm::vec4 col = c.At(idx);

                colors[colorIdx + 0] = RoundByteF(col.r);
                colors[colorIdx + 1] = RoundByteF(col.g);
                colors[colorIdx + 2] = RoundByteF(col.b);

                if (c.IsSolid(idx)) {
                    colors[colorIdx + 3] = 255;
                } else {
                    colors[colorIdx + 3] = 0;
                }
            }
        }
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_3D, tex);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
                 // GL_COMPRESSED_RGBA,
                 c.GetSize().x, c.GetSize().y, c.GetSize().z, 0, GL_RGBA, GL_UNSIGNED_BYTE, &colors[0]);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return tex;
}