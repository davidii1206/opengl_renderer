#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(ShaderStage shdaerType, const std::string &shaderSource) : target(
    shdaerType == ShaderStage::Vertex ? GL_VERTEX_SHADER :
    shdaerType == ShaderStage::Fragment ? GL_FRAGMENT_SHADER :
    shdaerType == ShaderStage::TessEval ? GL_TESS_EVALUATION_SHADER :
    shdaerType == ShaderStage::TessControl ? GL_TESS_CONTROL_SHADER :
    shdaerType == ShaderStage::Geometry ? GL_GEOMETRY_SHADER :
    GL_COMPUTE_SHADER
)
{
    std::string shaderString = ReadFile(shaderSource);
    const char* src = shaderString.c_str();
    id = glCreateShader(target);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    GLint isCompiled = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(id, maxLength, &maxLength, &errorLog[0]);

        glDeleteShader(id);
        return;
    }
}

Shader::~Shader()
{
    if (id != 0) {
        glDeleteShader(id);
    }
}

std::string Shader::ReadFile(const std::string& filepath)
{
    std::ifstream stream(filepath);
    if (!stream) {
        fprintf(stderr, "Could not open file: %s\n", filepath.c_str());
        return "";
    }
    std::string line;
    std::stringstream ss;

    while (getline(stream, line))
    {
        ss << line << "\n";
    }
    return ss.str();
}