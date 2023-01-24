#pragma once
#include <string>
#include <glm/glm.hpp>

class Shader {
   public:
    Shader(const char *vertexPath, const char *fragmentShader, const char *geometryFile = nullptr);
    Shader(const char *computePath);
    ~Shader();

    // TEMP -> Move to a utility file
    static std::string ReadFile(const char *filepath);

    unsigned int GetID() const { return m_ID; }

    void Bind() const;
    void Unbind() const;

    void Set1i(const char *name, int value) const;
    void Set1iv(const char *name, int *value, int length) const;
    void Set1u(const char *name, unsigned int value) const;
    void Set1uv(const char *name, unsigned int *value, int length) const;
    void Set1f(const char *name, float value) const;
    void SetVec2fv(const char *name, const glm::vec2 &value) const;
    void SetVec3fv(const char *name, const glm::vec3 &value) const;
    void SetVec4fv(const char *name, const glm::vec4 &value) const;
    void SetMat4fv(const char *name, const glm::mat4 &value, bool transpose = false) const;

   private:
    int GetLocation(const char *name) const;
    unsigned int LoadShader(unsigned int type, const char *filename);
    void LinkProgram(unsigned int vShader, unsigned int fShader, unsigned int gShader);
    void LinkProgram(unsigned int cShader);

   private:
    unsigned int m_ID;
};