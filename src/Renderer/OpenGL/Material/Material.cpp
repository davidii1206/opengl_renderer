#include "Material.h"
#include <iostream>
#include <stdexcept>

Material::Material(const std::string& name) : m_name(name) {
    // Set default PBR values
    SetVec4("u_baseColorFactor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    SetFloat("u_metallicFactor", 1.0f);
    SetFloat("u_roughnessFactor", 1.0f);
    SetVec3("u_emissiveFactor", glm::vec3(0.0f));
    SetFloat("u_normalScale", 1.0f);
    SetFloat("u_occlusionStrength", 1.0f);
    
    // Set texture availability flags to false by default
    SetBool("u_hasBaseColorTexture", false);
    SetBool("u_hasMetallicRoughnessTexture", false);
    SetBool("u_hasNormalTexture", false);
    SetBool("u_hasEmissiveTexture", false);
    SetBool("u_hasOcclusionTexture", false);
}

void Material::SetFloat(const std::string& name, float value) {
    m_parameters[name] = MaterialUniform(value);
}

void Material::SetVec2(const std::string& name, const glm::vec2& value) {
    m_parameters[name] = MaterialUniform(value);
}

void Material::SetVec3(const std::string& name, const glm::vec3& value) {
    m_parameters[name] = MaterialUniform(value);
}

void Material::SetVec4(const std::string& name, const glm::vec4& value) {
    m_parameters[name] = MaterialUniform(value);
}

void Material::SetInt(const std::string& name, int value) {
    m_parameters[name] = MaterialUniform(value);
}

void Material::SetBool(const std::string& name, bool value) {
    m_parameters[name] = MaterialUniform(value);
}

void Material::SetTexture(const std::string& name, std::shared_ptr<Texture> texture) {
    m_parameters[name] = MaterialUniform(texture);
    
    // Automatically set texture availability flags
    if (name == "u_baseColorTexture") {
        SetBool("u_hasBaseColorTexture", texture != nullptr);
    } else if (name == "u_metallicRoughnessTexture") {
        SetBool("u_hasMetallicRoughnessTexture", texture != nullptr);
    } else if (name == "u_normalTexture") {
        SetBool("u_hasNormalTexture", texture != nullptr);
    } else if (name == "u_emissiveTexture") {
        SetBool("u_hasEmissiveTexture", texture != nullptr);
    } else if (name == "u_occlusionTexture") {
        SetBool("u_hasOcclusionTexture", texture != nullptr);
    }
}

float Material::GetFloat(const std::string& name) const {
    auto it = m_parameters.find(name);
    if (it != m_parameters.end() && it->second.type == MaterialParameter::Float) {
        return it->second.floatValue;
    }
    return 0.0f;
}

glm::vec2 Material::GetVec2(const std::string& name) const {
    auto it = m_parameters.find(name);
    if (it != m_parameters.end() && it->second.type == MaterialParameter::Vec2) {
        return it->second.vec2Value;
    }
    return glm::vec2(0.0f);
}

glm::vec3 Material::GetVec3(const std::string& name) const {
    auto it = m_parameters.find(name);
    if (it != m_parameters.end() && it->second.type == MaterialParameter::Vec3) {
        return it->second.vec3Value;
    }
    return glm::vec3(0.0f);
}

glm::vec4 Material::GetVec4(const std::string& name) const {
    auto it = m_parameters.find(name);
    if (it != m_parameters.end() && it->second.type == MaterialParameter::Vec4) {
        return it->second.vec4Value;
    }
    return glm::vec4(0.0f);
}

int Material::GetInt(const std::string& name) const {
    auto it = m_parameters.find(name);
    if (it != m_parameters.end() && it->second.type == MaterialParameter::Int) {
        return it->second.intValue;
    }
    return 0;
}

bool Material::GetBool(const std::string& name) const {
    auto it = m_parameters.find(name);
    if (it != m_parameters.end() && it->second.type == MaterialParameter::Bool) {
        return it->second.boolValue;
    }
    return false;
}

std::shared_ptr<Texture> Material::GetTexture(const std::string& name) const {
    auto it = m_parameters.find(name);
    if (it != m_parameters.end() && it->second.type == MaterialParameter::Texture) {
        return it->second.textureValue;
    }
    return nullptr;
}

void Material::Bind() {
    if (!m_shader) {
        std::cerr << "Warning: Material '" << m_name << "' has no shader assigned!" << std::endl;
        return;
    }

    m_shader->useShaderProgram();
    
    // Reset texture unit counter for this draw call
    m_nextTextureUnit = 0;
    
    // Apply all parameters
    for (const auto& [name, uniform] : m_parameters) {
        switch (uniform.type) {
            case MaterialParameter::Float:
                m_shader->SetUniform1f(name, uniform.floatValue);
                break;
            case MaterialParameter::Vec2:
                m_shader->SetUniformVec2(name, uniform.vec2Value);
                break;
            case MaterialParameter::Vec3:
                m_shader->SetUniformVec3(name, uniform.vec3Value);
                break;
            case MaterialParameter::Vec4:
                m_shader->SetUniformVec4(name, uniform.vec4Value);
                break;
            case MaterialParameter::Int:
                m_shader->SetUniform1i(name, uniform.intValue);
                break;
            case MaterialParameter::Bool:
                m_shader->SetUniform1i(name, uniform.boolValue ? 1 : 0);
                break;
            case MaterialParameter::Texture:
                if (uniform.textureValue) {
                    // Assign texture unit
                    GLuint textureUnit = m_nextTextureUnit++;
                    uniform.textureValue->BindTextureForSampling(textureUnit);
                    m_shader->SetUniform1i(name, textureUnit);
                }
                break;
        }
    }
}