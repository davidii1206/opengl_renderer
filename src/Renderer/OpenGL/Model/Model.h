#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include "tiny_gltf.h"
#include "../Mesh/Mesh.h"
#include "../Material/Material.h"
#include "../Texture/Texture.h"

struct ModelNode {
    std::string name;
    glm::mat4 transform;
    std::vector<int> meshIndices;
    std::vector<std::unique_ptr<ModelNode>> children;
    
    ModelNode() : transform(1.0f) {}
};

class Model {
public:
    explicit Model(const std::string& filepath);
    ~Model() = default;

    // Delete copy constructor and assignment
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    // Move constructor and assignment
    Model(Model&& other) noexcept;
    Model& operator=(Model&& other) noexcept;

    // Render the entire model
    void Draw(const glm::mat4& modelMatrix = glm::mat4(1.0f)) const;
    
    // Set shader for all materials in the model
    void SetShaderForAllMaterials(std::shared_ptr<ShaderProgram> shader);

    // Getters
    const std::string& GetFilePath() const { return m_filepath; }
    size_t GetMeshCount() const { return m_meshes.size(); }
    size_t GetMaterialCount() const { return m_materials.size(); }
    
    // Get bounding box
    const glm::vec3& GetBoundingBoxMin() const { return m_boundingBoxMin; }
    const glm::vec3& GetBoundingBoxMax() const { return m_boundingBoxMax; }
    glm::vec3 GetBoundingBoxCenter() const { return (m_boundingBoxMin + m_boundingBoxMax) * 0.5f; }
    glm::vec3 GetBoundingBoxSize() const { return m_boundingBoxMax - m_boundingBoxMin; }

    // Public members (following your architecture pattern)
    std::vector<std::unique_ptr<Mesh>> m_meshes;
    std::vector<std::shared_ptr<Material>> m_materials;
    std::vector<std::shared_ptr<Texture>> m_textures;

private:
    std::string m_filepath;
    tinygltf::Model m_gltfModel;
    std::unique_ptr<ModelNode> m_rootNode;
    
    // Bounding box
    glm::vec3 m_boundingBoxMin;
    glm::vec3 m_boundingBoxMax;

    // Loading functions
    bool LoadModel(const std::string& filepath);
    void ProcessNode(const tinygltf::Node& gltfNode, ModelNode& node);
    std::unique_ptr<Mesh> ProcessMesh(const tinygltf::Mesh& gltfMesh);
    std::shared_ptr<Material> ProcessMaterial(const tinygltf::Material& gltfMaterial);
    std::shared_ptr<Texture> LoadTexture(const tinygltf::Image& image, const std::string& directory);
    std::shared_ptr<Texture> LoadTextureFromGLTF(int textureIndex);
    
    // Helper functions
    glm::mat4 GetNodeTransform(const tinygltf::Node& node);
    void ExtractVertexData(const tinygltf::Primitive& primitive, std::vector<Vertex3D>& vertices, std::vector<uint32_t>& indices);
    void UpdateBoundingBox(const glm::vec3& point);
    void CalculateBoundingBox();
    
    // Rendering
    void DrawNode(const ModelNode& node, const glm::mat4& parentTransform) const;

    // Texture cache to avoid loading same texture multiple times
    std::unordered_map<int, std::shared_ptr<Texture>> m_textureCache;
};

// Factory function
inline std::unique_ptr<Model> CreateModel(const std::string& filepath) {
    return std::make_unique<Model>(filepath);
}