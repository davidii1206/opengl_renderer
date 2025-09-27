#include "Mesh.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>

constexpr float PI = 3.14159265358979323846f;

Mesh::Mesh() : m_boundingBoxMin(FLT_MAX), m_boundingBoxMax(-FLT_MAX), m_boundingBoxCalculated(false) {
    // Empty mesh - will be initialized when data is set
}

Mesh::Mesh(const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices)
    : m_vertices(vertices), m_indices(indices), 
      m_boundingBoxMin(FLT_MAX), m_boundingBoxMax(-FLT_MAX), m_boundingBoxCalculated(false) {
    SetupMesh();
    CalculateBoundingBox();
}

Mesh::Mesh(Mesh&& other) noexcept
    : m_vertices(std::move(other.m_vertices)),
      m_indices(std::move(other.m_indices)),
      m_vao(std::move(other.m_vao)),
      m_vbo(std::move(other.m_vbo)),
      m_ebo(std::move(other.m_ebo)),
      m_defaultMaterial(std::move(other.m_defaultMaterial)),
      m_subMeshes(std::move(other.m_subMeshes)),
      m_boundingBoxMin(other.m_boundingBoxMin),
      m_boundingBoxMax(other.m_boundingBoxMax),
      m_boundingBoxCalculated(other.m_boundingBoxCalculated) {
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        m_vertices = std::move(other.m_vertices);
        m_indices = std::move(other.m_indices);
        m_vao = std::move(other.m_vao);
        m_vbo = std::move(other.m_vbo);
        m_ebo = std::move(other.m_ebo);
        m_defaultMaterial = std::move(other.m_defaultMaterial);
        m_subMeshes = std::move(other.m_subMeshes);
        m_boundingBoxMin = other.m_boundingBoxMin;
        m_boundingBoxMax = other.m_boundingBoxMax;
        m_boundingBoxCalculated = other.m_boundingBoxCalculated;
    }
    return *this;
}

void Mesh::SetVertices(const std::vector<Vertex3D>& vertices) {
    m_vertices = vertices;
    m_boundingBoxCalculated = false;
    UpdateBuffers();
    CalculateBoundingBox();
}

void Mesh::SetIndices(const std::vector<uint32_t>& indices) {
    m_indices = indices;
    UpdateBuffers();
}

void Mesh::SetVerticesAndIndices(const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices) {
    m_vertices = vertices;
    m_indices = indices;
    m_boundingBoxCalculated = false;
    SetupMesh();
    CalculateBoundingBox();
}

void Mesh::SetupMesh() {
    if (m_vertices.empty()) {
        spdlog::warn("Attempting to setup mesh with no vertices");
        return;
    }

    m_vao = CreateVertexArray();
    m_vao->desc = Vertex3D::GetVertexDescription();

    m_vbo = CreateBuffer(BufferType::Vertex, 
                         m_vertices.size() * sizeof(Vertex3D), 
                         m_vertices.data(), 
                         BufferUsage::Static);

    if (!m_indices.empty()) {
        m_ebo = CreateBuffer(BufferType::Index, 
                             m_indices.size() * sizeof(uint32_t), 
                             m_indices.data(), 
                             BufferUsage::Static);
    }

    m_vao->ApplyLayout(m_vbo->id);

    if (m_ebo) {
        m_vao->bind();
        m_ebo->bind();
    }
}

void Mesh::UpdateBuffers() {
    if (!m_vao) {
        SetupMesh();
        return;
    }

    if (m_vbo && !m_vertices.empty()) {
        if (m_vbo->size < m_vertices.size() * sizeof(Vertex3D)) {
            // Need to recreate buffer if it's too small
            m_vbo = CreateBuffer(BufferType::Vertex, 
                                 m_vertices.size() * sizeof(Vertex3D), 
                                 m_vertices.data(), 
                                 BufferUsage::Static);
            m_vao->ApplyLayout(m_vbo->id);
        } else {
            m_vbo->UpdateBuffer(m_vertices.data(), m_vertices.size() * sizeof(Vertex3D));
        }
    }

    if (!m_indices.empty()) {
        if (!m_ebo || m_ebo->size < m_indices.size() * sizeof(uint32_t)) {
            m_ebo = CreateBuffer(BufferType::Index, 
                                 m_indices.size() * sizeof(uint32_t), 
                                 m_indices.data(), 
                                 BufferUsage::Static);
            m_vao->bind();
            m_ebo->bind();
        } else {
            m_ebo->UpdateBuffer(m_indices.data(), m_indices.size() * sizeof(uint32_t));
        }
    }
}

void Mesh::AddSubmesh(uint32_t indexOffset, uint32_t indexCount, std::shared_ptr<Material> material, const std::string& name) {
    if (indexOffset + indexCount > m_indices.size()) {
        spdlog::error("Submesh indices out of range: offset={}, count={}, total={}", 
                      indexOffset, indexCount, m_indices.size());
        return;
    }
    
    m_subMeshes.emplace_back(indexOffset, indexCount, material, name);
    spdlog::debug("Added submesh '{}': offset={}, count={}, material={}", 
                  name, indexOffset, indexCount, material ? "yes" : "no");
}

void Mesh::Draw() const {
    if (!m_vao) {
        spdlog::error("Mesh VAO is not initialized!");
        return;
    }

    if (m_vertices.empty()) {
        spdlog::warn("Attempting to draw empty mesh");
        return;
    }

    m_vao->bind();

    if (m_subMeshes.empty()) {
        if (m_defaultMaterial) {
            m_defaultMaterial->Bind();
        }
        
        if (!m_indices.empty()) {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
        }
    } else {
        for (const auto& submesh : m_subMeshes) {
            if (submesh.indexCount == 0) continue;

            auto material = submesh.material ? submesh.material : m_defaultMaterial;
            if (material) {
                material->Bind();
            }

            if (!m_indices.empty()) {
                glDrawElements(GL_TRIANGLES, 
                              static_cast<GLsizei>(submesh.indexCount), 
                              GL_UNSIGNED_INT, 
                              reinterpret_cast<void*>(submesh.indexOffset * sizeof(uint32_t)));
            } else {
                glDrawArrays(GL_TRIANGLES, 
                            static_cast<GLint>(submesh.indexOffset),
                            static_cast<GLsizei>(submesh.indexCount));
            }
        }
    }
}

void Mesh::CalculateTangents() {
    if (m_vertices.empty() || m_indices.empty()) {
        spdlog::warn("Cannot calculate tangents: mesh has no vertices or indices");
        return;
    }

    for (auto& vertex : m_vertices) {
        vertex.tangent = glm::vec3(0.0f);
        vertex.bitangent = glm::vec3(0.0f);
    }

    for (size_t i = 0; i < m_indices.size(); i += 3) {
        if (i + 2 >= m_indices.size()) break;
        
        uint32_t i0 = m_indices[i];
        uint32_t i1 = m_indices[i + 1];
        uint32_t i2 = m_indices[i + 2];

        if (i0 >= m_vertices.size() || i1 >= m_vertices.size() || i2 >= m_vertices.size()) {
            spdlog::warn("Invalid indices in tangent calculation: {}, {}, {}", i0, i1, i2);
            continue;
        }

        Vertex3D& v0 = m_vertices[i0];
        Vertex3D& v1 = m_vertices[i1];
        Vertex3D& v2 = m_vertices[i2];

        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;

        glm::vec2 deltaUV1 = v1.texCoords - v0.texCoords;
        glm::vec2 deltaUV2 = v2.texCoords - v0.texCoords;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
   
        if (!std::isfinite(f)) {
            f = 0.0f;
        }

        glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
        glm::vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);

        v0.tangent += tangent;
        v1.tangent += tangent;
        v2.tangent += tangent;

        v0.bitangent += bitangent;
        v1.bitangent += bitangent;
        v2.bitangent += bitangent;
    }

    for (auto& vertex : m_vertices) {
        if (glm::length(vertex.tangent) > 0.0f) {
            vertex.tangent = glm::normalize(vertex.tangent);
            
            vertex.tangent = glm::normalize(vertex.tangent - vertex.normal * glm::dot(vertex.normal, vertex.tangent));
            
            if (glm::dot(glm::cross(vertex.normal, vertex.tangent), vertex.bitangent) < 0.0f) {
                vertex.tangent = -vertex.tangent;
            }
        } else {
            vertex.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
        }

        if (glm::length(vertex.bitangent) > 0.0f) {
            vertex.bitangent = glm::normalize(vertex.bitangent);
        } else {
            vertex.bitangent = glm::cross(vertex.normal, vertex.tangent);
        }
    }

    UpdateBuffers();
    spdlog::debug("Calculated tangents for {} vertices", m_vertices.size());
}

void Mesh::CalculateBoundingBox() {
    if (m_vertices.empty()) {
        m_boundingBoxMin = glm::vec3(0.0f);
        m_boundingBoxMax = glm::vec3(0.0f);
        m_boundingBoxCalculated = true;
        return;
    }

    m_boundingBoxMin = glm::vec3(FLT_MAX);
    m_boundingBoxMax = glm::vec3(-FLT_MAX);
    
    for (const auto& vertex : m_vertices) {
        UpdateBoundingBox(vertex.position);
    }
    
    m_boundingBoxCalculated = true;
    
    spdlog::debug("Calculated bounding box: min({:.2f}, {:.2f}, {:.2f}), max({:.2f}, {:.2f}, {:.2f})",
                 m_boundingBoxMin.x, m_boundingBoxMin.y, m_boundingBoxMin.z,
                 m_boundingBoxMax.x, m_boundingBoxMax.y, m_boundingBoxMax.z);
}

void Mesh::UpdateBoundingBox(const glm::vec3& point) {
    m_boundingBoxMin = glm::min(m_boundingBoxMin, point);
    m_boundingBoxMax = glm::max(m_boundingBoxMax, point);
}

bool Mesh::IsValid() const {
    if (m_vertices.empty()) {
        spdlog::warn("Mesh validation failed: no vertices");
        return false;
    }

    if (!m_indices.empty()) {
        for (uint32_t index : m_indices) {
            if (index >= m_vertices.size()) {
                spdlog::error("Mesh validation failed: index {} out of range (vertex count: {})", 
                             index, m_vertices.size());
                return false;
            }
        }
        
        if (m_indices.size() % 3 != 0) {
            spdlog::warn("Mesh validation warning: index count {} is not divisible by 3", m_indices.size());
        }
    }

    // Validate submeshes
    for (size_t i = 0; i < m_subMeshes.size(); ++i) {
        const auto& submesh = m_subMeshes[i];
        if (submesh.indexOffset + submesh.indexCount > m_indices.size()) {
            spdlog::error("Mesh validation failed: submesh {} indices out of range", i);
            return false;
        }
    }

    return true;
}

namespace MeshUtils {
    
std::unique_ptr<Mesh> CreateCube(float size) {
    float halfSize = size * 0.5f;
    
    std::vector<Vertex3D> vertices = {
        Vertex3D(glm::vec3(-halfSize, -halfSize,  halfSize), glm::vec3(0, 0, 1), glm::vec2(0, 0)),
        Vertex3D(glm::vec3( halfSize, -halfSize,  halfSize), glm::vec3(0, 0, 1), glm::vec2(1, 0)),
        Vertex3D(glm::vec3( halfSize,  halfSize,  halfSize), glm::vec3(0, 0, 1), glm::vec2(1, 1)),
        Vertex3D(glm::vec3(-halfSize,  halfSize,  halfSize), glm::vec3(0, 0, 1), glm::vec2(0, 1)),
        
        Vertex3D(glm::vec3(-halfSize, -halfSize, -halfSize), glm::vec3(0, 0, -1), glm::vec2(1, 0)),
        Vertex3D(glm::vec3(-halfSize,  halfSize, -halfSize), glm::vec3(0, 0, -1), glm::vec2(1, 1)),
        Vertex3D(glm::vec3( halfSize,  halfSize, -halfSize), glm::vec3(0, 0, -1), glm::vec2(0, 1)),
        Vertex3D(glm::vec3( halfSize, -halfSize, -halfSize), glm::vec3(0, 0, -1), glm::vec2(0, 0)),
        
        Vertex3D(glm::vec3(-halfSize,  halfSize, -halfSize), glm::vec3(0, 1, 0), glm::vec2(0, 1)),
        Vertex3D(glm::vec3(-halfSize,  halfSize,  halfSize), glm::vec3(0, 1, 0), glm::vec2(0, 0)),
        Vertex3D(glm::vec3( halfSize,  halfSize,  halfSize), glm::vec3(0, 1, 0), glm::vec2(1, 0)),
        Vertex3D(glm::vec3( halfSize,  halfSize, -halfSize), glm::vec3(0, 1, 0), glm::vec2(1, 1)),
        
        Vertex3D(glm::vec3(-halfSize, -halfSize, -halfSize), glm::vec3(0, -1, 0), glm::vec2(1, 1)),
        Vertex3D(glm::vec3( halfSize, -halfSize, -halfSize), glm::vec3(0, -1, 0), glm::vec2(0, 1)),
        Vertex3D(glm::vec3( halfSize, -halfSize,  halfSize), glm::vec3(0, -1, 0), glm::vec2(0, 0)),
        Vertex3D(glm::vec3(-halfSize, -halfSize,  halfSize), glm::vec3(0, -1, 0), glm::vec2(1, 0)),
        
        Vertex3D(glm::vec3( halfSize, -halfSize, -halfSize), glm::vec3(1, 0, 0), glm::vec2(1, 0)),
        Vertex3D(glm::vec3( halfSize,  halfSize, -halfSize), glm::vec3(1, 0, 0), glm::vec2(1, 1)),
        Vertex3D(glm::vec3( halfSize,  halfSize,  halfSize), glm::vec3(1, 0, 0), glm::vec2(0, 1)),
        Vertex3D(glm::vec3( halfSize, -halfSize,  halfSize), glm::vec3(1, 0, 0), glm::vec2(0, 0)),
        
        Vertex3D(glm::vec3(-halfSize, -halfSize, -halfSize), glm::vec3(-1, 0, 0), glm::vec2(0, 0)),
        Vertex3D(glm::vec3(-halfSize, -halfSize,  halfSize), glm::vec3(-1, 0, 0), glm::vec2(1, 0)),
        Vertex3D(glm::vec3(-halfSize,  halfSize,  halfSize), glm::vec3(-1, 0, 0), glm::vec2(1, 1)),
        Vertex3D(glm::vec3(-halfSize,  halfSize, -halfSize), glm::vec3(-1, 0, 0), glm::vec2(0, 1))
    };
    
    std::vector<uint32_t> indices = {
        0,  1,  2,   2,  3,  0,   
        4,  5,  6,   6,  7,  4,  
        8,  9,  10,  10, 11, 8,  
        12, 13, 14,  14, 15, 12,  
        16, 17, 18,  18, 19, 16, 
        20, 21, 22,  22, 23, 20  
    };
    
    auto mesh = CreateMesh(vertices, indices);
    mesh->CalculateTangents();
    return mesh;
}

std::unique_ptr<Mesh> CreateSphere(float radius, int segments) {
    std::vector<Vertex3D> vertices;
    std::vector<uint32_t> indices;
    
    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * PI / segments;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);
        
        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2 * PI / segments;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);
            
            glm::vec3 position(radius * sinTheta * cosPhi, radius * cosTheta, radius * sinTheta * sinPhi);
            glm::vec3 normal = glm::normalize(position);
            glm::vec2 texCoords(static_cast<float>(lon) / segments, static_cast<float>(lat) / segments);
            
            vertices.emplace_back(position, normal, texCoords);
        }
    }
    
    for (int lat = 0; lat < segments; ++lat) {
        for (int lon = 0; lon < segments; ++lon) {
            uint32_t first = lat * (segments + 1) + lon;
            uint32_t second = first + segments + 1;
            
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
    
    auto mesh = CreateMesh(vertices, indices);
    mesh->CalculateTangents();
    return mesh;
}

std::unique_ptr<Mesh> CreatePlane(float width, float height) {
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    
    std::vector<Vertex3D> vertices = {
        Vertex3D(glm::vec3(-halfWidth, 0, -halfHeight), glm::vec3(0, 1, 0), glm::vec2(0, 0)),
        Vertex3D(glm::vec3( halfWidth, 0, -halfHeight), glm::vec3(0, 1, 0), glm::vec2(1, 0)),
        Vertex3D(glm::vec3( halfWidth, 0,  halfHeight), glm::vec3(0, 1, 0), glm::vec2(1, 1)),
        Vertex3D(glm::vec3(-halfWidth, 0,  halfHeight), glm::vec3(0, 1, 0), glm::vec2(0, 1))
    };
    
    std::vector<uint32_t> indices = {
        0, 1, 2,
        2, 3, 0
    };
    
    auto mesh = CreateMesh(vertices, indices);
    mesh->CalculateTangents();
    return mesh;
}

std::unique_ptr<Mesh> CreateQuad() {
    std::vector<Vertex3D> vertices = {
        Vertex3D(glm::vec3(-1, -1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0)),
        Vertex3D(glm::vec3( 1, -1, 0), glm::vec3(0, 0, 1), glm::vec2(1, 0)),
        Vertex3D(glm::vec3( 1,  1, 0), glm::vec3(0, 0, 1), glm::vec2(1, 1)),
        Vertex3D(glm::vec3(-1,  1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 1))
    };
    
    std::vector<uint32_t> indices = {
        0, 1, 2,
        2, 3, 0
    };
    
    auto mesh = CreateMesh(vertices, indices);
    mesh->CalculateTangents();
    return mesh;
}

} 