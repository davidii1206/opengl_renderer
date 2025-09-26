#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include "../Shader/ShaderProgram.h"
#include "../Texture/Texture.h"

enum class MaterialParameter {
    Float,
    Vec2,
    Vec3,
    Vec4,
    Int,
    Bool,
    Texture
};

struct MaterialUniform {
    MaterialParameter type;
    union {
        float floatValue;
        glm::vec2 vec2Value;
        glm::vec3 vec3Value;
        glm::vec4 vec4Value;
        int intValue;
        bool boolValue;
    };
    std::shared_ptr<Texture> textureValue;
    
    MaterialUniform() = default;
    MaterialUniform(float value) : type(MaterialParameter::Float), floatValue(value) {}
    MaterialUniform(const glm::vec2& value) : type(MaterialParameter::Vec2), vec2Value(value) {}
    MaterialUniform(const glm::vec3& value) : type(MaterialParameter::Vec3), vec3Value(value) {}
    MaterialUniform(const glm::vec4& value) : type(MaterialParameter::Vec4), vec4Value(value) {}
    MaterialUniform(int value) : type(MaterialParameter::Int), intValue(value) {}
    MaterialUniform(bool value) : type(MaterialParameter::Bool), boolValue(value) {}
    MaterialUniform(std::shared_ptr<Texture> texture) : type(MaterialParameter::Texture), textureValue(texture) {}
};

class Material {
public:
    Material(const std::string& name = "DefaultMaterial");
    ~Material() = default;

    // Shader management
    void SetShader(std::shared_ptr<ShaderProgram> shader) { m_shader = shader; }
    std::shared_ptr<ShaderProgram> GetShader() const { return m_shader; }

    // Parameter setters
    void SetFloat(const std::string& name, float value);
    void SetVec2(const std::string& name, const glm::vec2& value);
    void SetVec3(const std::string& name, const glm::vec3& value);
    void SetVec4(const std::string& name, const glm::vec4& value);
    void SetInt(const std::string& name, int value);
    void SetBool(const std::string& name, bool value);
    void SetTexture(const std::string& name, std::shared_ptr<Texture> texture);

    // Parameter getters
    float GetFloat(const std::string& name) const;
    glm::vec2 GetVec2(const std::string& name) const;
    glm::vec3 GetVec3(const std::string& name) const;
    glm::vec4 GetVec4(const std::string& name) const;
    int GetInt(const std::string& name) const;
    bool GetBool(const std::string& name) const;
    std::shared_ptr<Texture> GetTexture(const std::string& name) const;

    // Apply all material parameters to the shader
    void Bind();

    // Getters
    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }

    // Common PBR material properties (convenience methods)
    void SetBaseColor(const glm::vec4& color) { SetVec4("u_baseColorFactor", color); }
    void SetBaseColorTexture(std::shared_ptr<Texture> texture) { SetTexture("u_baseColorTexture", texture); }
    void SetMetallicFactor(float metallic) { SetFloat("u_metallicFactor", metallic); }
    void SetRoughnessFactor(float roughness) { SetFloat("u_roughnessFactor", roughness); }
    void SetMetallicRoughnessTexture(std::shared_ptr<Texture> texture) { SetTexture("u_metallicRoughnessTexture", texture); }
    void SetNormalTexture(std::shared_ptr<Texture> texture) { SetTexture("u_normalTexture", texture); }
    void SetEmissiveFactor(const glm::vec3& emissive) { SetVec3("u_emissiveFactor", emissive); }
    void SetEmissiveTexture(std::shared_ptr<Texture> texture) { SetTexture("u_emissiveTexture", texture); }

private:
    std::string m_name;
    std::shared_ptr<ShaderProgram> m_shader;
    std::unordered_map<std::string, MaterialUniform> m_parameters;
    mutable std::unordered_map<std::string, GLuint> m_textureUnits;
    mutable GLuint m_nextTextureUnit = 0;
};

// Factory function
inline std::shared_ptr<Material> CreateMaterial(const std::string& name = "DefaultMaterial") {
    return std::make_shared<Material>(name);
}