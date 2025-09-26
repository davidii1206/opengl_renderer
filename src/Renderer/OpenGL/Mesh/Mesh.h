#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "../Material/Material.h"
#include "../Buffer/Buffer.h"
#include "../Vertex/VertexArray.h"

// Standard vertex structure for 3D models
struct Vertex3D {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    
    // Default constructor
    Vertex3D() 
        : position(0.0f), normal(0.0f, 0.0f, 1.0f), texCoords(0.0f), 
          tangent(1.0f, 0.0f, 0.0f), bitangent(0.0f, 1.0f, 0.0f) {}
    
    // Constructor with position only
    Vertex3D(const glm::vec3& pos) 
        : position(pos), normal(0.0f, 0.0f, 1.0f), texCoords(0.0f), 
          tangent(1.0f, 0.0f, 0.0f), bitangent(0.0f, 1.0f, 0.0f) {}
    
    // Constructor with position and normal
    Vertex3D(const glm::vec3& pos, const glm::vec3& norm) 
        : position(pos), normal(norm), texCoords(0.0f), 
          tangent(1.0f, 0.0f, 0.0f), bitangent(0.0f, 1.0f, 0.0f) {}
    
    // Full constructor
    Vertex3D(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& tex)
        : position(pos), normal(norm), texCoords(tex), 
          tangent(1.0f, 0.0f, 0.0f), bitangent(0.0f, 1.0f, 0.0f) {}
    
    static VertexDescription GetVertexDescription() {
        return VertexDescription{
            sizeof(Vertex3D),
            {
                {0, 3, GL_FLOAT, offsetof(Vertex3D, position), false},   // position
                {1, 3, GL_FLOAT, offsetof(Vertex3D, normal), false},     // normal
                {2, 2, GL_FLOAT, offsetof(Vertex3D, texCoords), false},  // texCoords
                {3, 3, GL_FLOAT, offsetof(Vertex3D, tangent), false},    // tangent
                {4, 3, GL_FLOAT, offsetof(Vertex3D, bitangent), false}   // bitangent
            }
        };
    }
};

// Submesh represents a portion of mesh with a specific material
struct Submesh {
    uint32_t indexOffset;    // Start index in the index buffer
    uint32_t indexCount;     // Number of indices for this submesh
    std::shared_ptr<Material> material;
    std::string name;        // Optional name for debugging
    
    Submesh() : indexOffset(0), indexCount(0), material(nullptr) {}
    
    Submesh(uint32_t offset, uint32_t count, std::shared_ptr<Material> mat = nullptr, const std::string& submeshName = "")
        : indexOffset(offset), indexCount(count), material(mat), name(submeshName) {}
};

class Mesh {
public:
    Mesh();
    Mesh(const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices);
    ~Mesh() = default;

    // Delete copy constructor and assignment
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Move constructor and assignment
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    // Vertex and index data management
    void SetVertices(const std::vector<Vertex3D>& vertices);
    void SetIndices(const std::vector<uint32_t>& indices);
    void SetVerticesAndIndices(const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices);

    // Add a submesh
    void AddSubmesh(uint32_t indexOffset, uint32_t indexCount, std::shared_ptr<Material> material = nullptr, const std::string& name = "");
    void ClearSubmeshes() { m_subMeshes.clear(); }
    
    // Set default material (used when submesh has no material)
    void SetDefaultMaterial(std::shared_ptr<Material> material) { m_defaultMaterial = material; }
    std::shared_ptr<Material> GetDefaultMaterial() const { return m_defaultMaterial; }

    // Render the mesh
    void Draw() const;
    
    // Getters
    const std::vector<Vertex3D>& GetVertices() const { return m_vertices; }
    const std::vector<uint32_t>& GetIndices() const { return m_indices; }
    size_t GetVertexCount() const { return m_vertices.size(); }
    size_t GetIndexCount() const { return m_indices.size(); }
    size_t GetSubmeshCount() const { return m_subMeshes.size(); }

    // Calculate tangents and bitangents
    void CalculateTangents();
    
    // Calculate bounding box
    void CalculateBoundingBox();
    const glm::vec3& GetBoundingBoxMin() const { return m_boundingBoxMin; }
    const glm::vec3& GetBoundingBoxMax() const { return m_boundingBoxMax; }
    glm::vec3 GetBoundingBoxCenter() const { return (m_boundingBoxMin + m_boundingBoxMax) * 0.5f; }
    glm::vec3 GetBoundingBoxSize() const { return m_boundingBoxMax - m_boundingBoxMin; }

    // Validate mesh data
    bool IsValid() const;

    // Public member for submesh access (following your architecture pattern)
    std::vector<Submesh> m_subMeshes;

private:
    std::vector<Vertex3D> m_vertices;
    std::vector<uint32_t> m_indices;
    
    std::unique_ptr<VertexArray> m_vao;
    std::unique_ptr<Buffer> m_vbo;
    std::unique_ptr<Buffer> m_ebo;
    
    std::shared_ptr<Material> m_defaultMaterial;
    
    // Bounding box
    glm::vec3 m_boundingBoxMin;
    glm::vec3 m_boundingBoxMax;
    bool m_boundingBoxCalculated;

    void SetupMesh();
    void UpdateBuffers();
    void UpdateBoundingBox(const glm::vec3& point);
};

// Factory functions
inline std::unique_ptr<Mesh> CreateMesh() {
    return std::make_unique<Mesh>();
}

inline std::unique_ptr<Mesh> CreateMesh(const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices) {
    return std::make_unique<Mesh>(vertices, indices);
}

// Utility functions for creating basic meshes
namespace MeshUtils {
    std::unique_ptr<Mesh> CreateCube(float size = 1.0f);
    std::unique_ptr<Mesh> CreateSphere(float radius = 1.0f, int segments = 32);
    std::unique_ptr<Mesh> CreatePlane(float width = 1.0f, float height = 1.0f);
    std::unique_ptr<Mesh> CreateQuad();
}