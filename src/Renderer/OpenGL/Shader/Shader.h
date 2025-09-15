#pragma once
#include "glad/glad.h"
#include <string>
#include <cstdint>
#include <vector>
#include <memory>

enum class ShaderStage { Vertex, Fragment, Geometry, TessControl, TessEval, Compute };


class Shader {
    public:
        Shader(ShaderStage shaderType, const std::string &shaderSource);
        ~Shader();

        std::string ReadFile(const std::string& filepath);

        uint32_t id;
        const GLenum target;
};

inline std::unique_ptr<Shader> CreateShader (ShaderStage shaderType, const std::string &shaderSource) {
    return std::make_unique<Shader>(shaderType, shaderSource);
}