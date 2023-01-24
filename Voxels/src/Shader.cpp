#include "Shader.hpp"
#include <fstream>
#include <glad/glad.h>

Shader::Shader(const char *vertexPath, const char *fragmentPath, const char *geometryPath) {
    unsigned int vShader = 0, fShader = 0, gShader = 0;
    vShader = LoadShader(GL_VERTEX_SHADER, vertexPath);
    fShader = LoadShader(GL_FRAGMENT_SHADER, fragmentPath);
    if (geometryPath)
        gShader = LoadShader(GL_GEOMETRY_SHADER, geometryPath);

    LinkProgram(vShader, fShader, gShader);
}

Shader::Shader(const char *computePath) {
    unsigned int cShader = LoadShader(GL_COMPUTE_SHADER, computePath);
    LinkProgram(cShader);
}

Shader::~Shader() {
    glDeleteProgram(m_ID);
}

void Shader::Bind() const {
    glUseProgram(m_ID);
}
void Shader::Unbind() const {
    glUseProgram(0);
}

void Shader::Set1i(const char *name, int value) const {
    glUniform1i(GetLocation(name), value);
}

void Shader::Set1iv(const char *name, int *value, int length) const {
    glUniform1iv(GetLocation(name), length, value);
}

void Shader::Set1u(const char *name, unsigned int value) const {
    glUniform1ui(GetLocation(name), value);
}

void Shader::Set1uv(const char *name, unsigned int *value, int length) const {
    glUniform1uiv(GetLocation(name), length, value);
}

void Shader::Set1f(const char *name, float value) const {
    glUniform1f(GetLocation(name), value);
}

void Shader::SetVec2fv(const char *name, const glm::vec2 &value) const {
    glUniform2fv(GetLocation(name), 1, &value[0]);
}

void Shader::SetVec3fv(const char *name, const glm::vec3 &value) const {
    glUniform3fv(GetLocation(name), 1, &value[0]);
}

void Shader::SetVec4fv(const char *name, const glm::vec4 &value) const {
    glUniform4fv(GetLocation(name), 1, &value[0]);
}

void Shader::SetMat4fv(const char *name, const glm::mat4 &value, bool transpose) const {
    glUniformMatrix4fv(GetLocation(name), 1, transpose, &value[0][0]);
}

int Shader::GetLocation(const char *name) const {
    return glGetUniformLocation(m_ID, name);
}

unsigned int Shader::LoadShader(unsigned int type, const char *filename) {
    int success;
    char infoLog[512];

    unsigned int shader = glCreateShader(type);
    std::string str_src = ReadFile(filename).c_str();
    const char *source = str_src.c_str();

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        const char *shaderType = type == GL_VERTEX_SHADER     ? "VERTEX_SHADER"
                                 : type == GL_FRAGMENT_SHADER ? "FRAGMENT_SHADER"
                                 : type == GL_GEOMETRY_SHADER ? "GEOMETRY_SHADER"
                                                              : "COMPUTE_SHADER";
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::printf("ERROR::LOADSHADERS::COULD_NOT_COMPILE_%s\n", shaderType);
        std::printf("%s\n", infoLog);
        return 0;
    }
    return shader;
}

void Shader::LinkProgram(unsigned int vShader, unsigned int fShader, unsigned int gShader) {
    int success;
    char infoLog[512];

    m_ID = glCreateProgram();

    glAttachShader(m_ID, vShader);
    glAttachShader(m_ID, fShader);
    if (gShader)
        glAttachShader(m_ID, gShader);

    glLinkProgram(m_ID);

    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_ID, 512, NULL, infoLog);
        std::printf("ERROR::LOADSHADERS::COULD_NOT_LINK_PROGRAM\n");
        std::printf("%s", infoLog);
        return;
    }

    glDeleteShader(vShader);
    glDeleteShader(gShader);
    glDeleteShader(fShader);
}

void Shader::LinkProgram(unsigned int cShader) {
    int success;
    char infoLog[512];

    m_ID = glCreateProgram();

    glAttachShader(m_ID, cShader);

    glLinkProgram(m_ID);

    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_ID, 512, NULL, infoLog);
        std::printf("ERROR::LOADSHADERS::COULD_NOT_LINK_PROGRAM\n");
        std::printf("%s", infoLog);
        return;
    }

    glDeleteShader(cShader);
}

std::string Shader::ReadFile(const char *filepath) {
    std::FILE *fp = std::fopen(filepath, "rb");
    if (fp) {
        std::string contents;
        std::fseek(fp, 0, SEEK_END);
        contents.resize(std::ftell(fp));
        std::rewind(fp);
        std::fread(&contents[0], 1, contents.size(), fp);
        std::fclose(fp);
        return contents;
    } else {
        std::printf("Error: failed to load %s", filepath);
        return nullptr;
    }
};