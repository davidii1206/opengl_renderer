#pragma once
#include "glad/glad.h"
#include "Shader.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <cstdint>
#include <vector>
#include <memory>

class ShaderProgram {
    public:
    ShaderProgram(const std::vector<Shader*>& shaders);
    ShaderProgram(const Shader& shader);
    ~ShaderProgram();

    void useShaderProgram() const { glUseProgram(program); }

    void DispatchCompute() { glDispatchCompute(groupsX, groupsY, groupsZ); };

    void SetUniform1i(const std::string& name, int value);
    void SetUniform1f(const std::string& name, float value);
    void SetUniform2f(const std::string& name, float v0, float v1);
    void SetUniform3f(const std::string& name, float v0, float v1, float v2);
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
    void SetUniformVec2(const std::string& name, const glm::vec2& vector);
    void SetUniformVec3(const std::string& name, const glm::vec3& vector);
    void SetUniformVec4(const std::string& name, const glm::vec4& vector);
    void SetUniformMat3(const std::string& name, const glm::mat3& matrix);
    void SetUniformMat4(const std::string& name, const glm::mat4& matrix);


    uint32_t program;

    GLuint groupsX = 0;
    GLuint groupsY = 0;
    GLuint groupsZ = 0;
};

inline std::unique_ptr<ShaderProgram> CreateShaderProgram(const std::vector<Shader*>& shaders) {
    return std::make_unique<ShaderProgram>(shaders);
}

inline std::unique_ptr<ShaderProgram> CreateShaderProgram(const Shader& shader) {
    return std::make_unique<ShaderProgram>(shader);
}