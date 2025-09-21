#include "ShaderProgram.h"
#include <iostream>

ShaderProgram::ShaderProgram(const std::vector<Shader*>& shaders) {
    program = glCreateProgram();

    for (const auto& shader : shaders)
        glAttachShader(program, shader->id);


    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<GLchar> log(logLength);
        glGetProgramInfoLog(program, logLength, nullptr, log.data());

        glDeleteProgram(program);
        throw std::runtime_error("Program linking failed:\n" + std::string(log.begin(), log.end()));
    }

    for (const auto& shader : shaders) {
        glDetachShader(program, shader->id);
    }
}

ShaderProgram::ShaderProgram(const Shader& shader) {
    program = glCreateProgram();

    glAttachShader(program, shader.id);

    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<GLchar> log(logLength);
        glGetProgramInfoLog(program, logLength, nullptr, log.data());

        glDeleteProgram(program);
        throw std::runtime_error("Program linking failed:\n" + std::string(log.begin(), log.end()));
    }

    glDetachShader(program, shader.id);
}

ShaderProgram::~ShaderProgram() {
    if (program != 0) {
        glDeleteProgram(program);
    }
}

void ShaderProgram::SetUniform1i(const std::string& name, int value) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glProgramUniform1i(program, loc, value);
}

void ShaderProgram::SetUniform1f(const std::string& name, float value) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glProgramUniform1f(program, loc, value);
}

void ShaderProgram::SetUniform2f(const std::string& name, float v0, float v1) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glProgramUniform2f(program, loc, v0, v1);
}

void ShaderProgram::SetUniform3f(const std::string& name, float v0, float v1, float v2) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glProgramUniform3f(program, loc, v0, v1, v2);
}

void ShaderProgram::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glProgramUniform4f(program, loc, v0, v1, v2, v3);
}

void ShaderProgram::SetUniformVec2(const std::string& name, const glm::vec2& vector) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glProgramUniform2fv(program, loc, 1, &vector[0]);
}

void ShaderProgram::SetUniformVec3(const std::string& name, const glm::vec3& vector) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glProgramUniform3fv(program, loc, 1, &vector[0]);   
}

void ShaderProgram::SetUniformVec4(const std::string& name, const glm::vec4& vector) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glProgramUniform4fv(program, loc, 1, &vector[0]);
}

void ShaderProgram::SetUniformMat3(const std::string& name, const glm::mat3& matrix) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glProgramUniformMatrix3fv(program, loc, 1, GL_FALSE, glm::value_ptr(matrix));
}

void ShaderProgram::SetUniformMat4(const std::string& name, const glm::mat4& matrix) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glProgramUniformMatrix4fv(program, loc, 1, GL_FALSE, glm::value_ptr(matrix));
}

