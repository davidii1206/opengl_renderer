#include "Model.h"
#include <iostream>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <algorithm>

Model::Model(const std::string& filepath) 
    : m_filepath(filepath), m_boundingBoxMin(FLT_MAX), m_boundingBoxMax(-FLT_MAX) {
    if (!LoadModel(filepath)) {
        throw std::runtime_error("Failed to load model: " + filepath);
    }
}

Model::Model(Model&& other) noexcept
    : m_filepath(std::move(other.m_filepath)),
      m_gltfModel(std::move(other.m_gltfModel)),
      m_meshes(std::move(other.m_meshes)),
      m_materials(std::move(other.m_materials)),
      m_textures(std::move(other.m_textures)),
      m_rootNode(std::move(other.m_rootNode)),
      m_boundingBoxMin(other.m_boundingBoxMin),
      m_boundingBoxMax(other.m_boundingBoxMax),
      m_textureCache(std::move(other.m_textureCache)) {
}

Model& Model::operator=(Model&& other) noexcept {
    if (this != &other) {
        m_filepath = std::move(other.m_filepath);
        m_gltfModel = std::move(other.m_gltfModel);
        m_meshes = std::move(other.m_meshes);
        m_materials = std::move(other.m_materials);
        m_textures = std::move(other.m_textures);
        m_rootNode = std::move(other.m_rootNode);
        m_boundingBoxMin = other.m_boundingBoxMin;
        m_boundingBoxMax = other.m_boundingBoxMax;
        m_textureCache = std::move(other.m_textureCache);
    }
    return *this;
}

bool Model::LoadModel(const std::string& filepath) {
    tinygltf::TinyGLTF loader;
    std::string error, warning;

    std::filesystem::path path(filepath);
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    bool success = false;
    if (extension == ".gltf") {
        success = loader.LoadASCIIFromFile(&m_gltfModel, &error, &warning, filepath);
    } else if (extension == ".glb") {
        success = loader.LoadBinaryFromFile(&m_gltfModel, &error, &warning, filepath);
    } else {
        spdlog::error("Unsupported model format: {}", extension);
        return false;
    }

    if (!warning.empty()) {
        spdlog::warn("glTF warning: {}", warning);
    }

    if (!error.empty()) {
        spdlog::error("glTF error: {}", error);
        return false;
    }

    if (!success) {
        spdlog::error("Failed to load glTF model: {}", filepath);
        return false;
    }

    spdlog::info("Successfully loaded glTF model: {}", filepath);
    spdlog::info("  Meshes: {}", m_gltfModel.meshes.size());
    spdlog::info("  Materials: {}", m_gltfModel.materials.size());
    spdlog::info("  Textures: {}", m_gltfModel.textures.size());
    spdlog::info("  Images: {}", m_gltfModel.images.size());

    for (const auto& gltfMaterial : m_gltfModel.materials) {
        m_materials.push_back(ProcessMaterial(gltfMaterial));
    }

    for (const auto& gltfMesh : m_gltfModel.meshes) {
        auto mesh = ProcessMesh(gltfMesh);
        if (mesh) {
            m_meshes.push_back(std::move(mesh));
        }
    }

    m_rootNode = std::make_unique<ModelNode>();
    if (!m_gltfModel.scenes.empty()) {
        const auto& scene = m_gltfModel.scenes[m_gltfModel.defaultScene >= 0 ? m_gltfModel.defaultScene : 0];
        for (int nodeIndex : scene.nodes) {
            auto childNode = std::make_unique<ModelNode>();
            ProcessNode(m_gltfModel.nodes[nodeIndex], *childNode);
            m_rootNode->children.push_back(std::move(childNode));
        }
    }

    CalculateBoundingBox();
    return true;
}

void Model::ProcessNode(const tinygltf::Node& gltfNode, ModelNode& node) {
    node.name = gltfNode.name;
    node.transform = GetNodeTransform(gltfNode);

    if (gltfNode.mesh >= 0) {
        node.meshIndices.push_back(gltfNode.mesh);
    }

    for (int childIndex : gltfNode.children) {
        auto childNode = std::make_unique<ModelNode>();
        ProcessNode(m_gltfModel.nodes[childIndex], *childNode);
        node.children.push_back(std::move(childNode));
    }
}

std::unique_ptr<Mesh> Model::ProcessMesh(const tinygltf::Mesh& gltfMesh) {
    std::vector<Vertex3D> vertices;
    std::vector<uint32_t> indices;
    
    for (const auto& primitive : gltfMesh.primitives) {
        size_t vertexOffset = vertices.size();
        
        ExtractVertexData(primitive, vertices, indices);
        
        size_t indexCount = indices.size() - (vertexOffset > 0 ? m_meshes.empty() ? 0 : 
                          m_meshes.back()->GetIndexCount() : 0);
    }

    if (vertices.empty()) {
        spdlog::warn("Mesh '{}' has no vertices", gltfMesh.name);
        return nullptr;
    }

    auto mesh = CreateMesh(vertices, indices);
    
    size_t indexOffset = 0;
    for (size_t i = 0; i < gltfMesh.primitives.size(); ++i) {
        const auto& primitive = gltfMesh.primitives[i];

        size_t indexCount = 0;
        if (primitive.indices >= 0) {
            const auto& accessor = m_gltfModel.accessors[primitive.indices];
            indexCount = accessor.count;
        }
        
        std::shared_ptr<Material> material = nullptr;
        if (primitive.material >= 0 && primitive.material < m_materials.size()) {
            material = m_materials[primitive.material];
        }
        
        mesh->AddSubmesh(static_cast<uint32_t>(indexOffset), static_cast<uint32_t>(indexCount), material);
        indexOffset += indexCount;
    }

    mesh->CalculateTangents();
    
    return mesh;
}

std::shared_ptr<Material> Model::ProcessMaterial(const tinygltf::Material& gltfMaterial) {
    auto material = CreateMaterial(gltfMaterial.name.empty() ? "Unnamed" : gltfMaterial.name);

    if (gltfMaterial.pbrMetallicRoughness.baseColorFactor.size() == 4) {
        material->SetBaseColor(glm::vec4(
            gltfMaterial.pbrMetallicRoughness.baseColorFactor[0],
            gltfMaterial.pbrMetallicRoughness.baseColorFactor[1],
            gltfMaterial.pbrMetallicRoughness.baseColorFactor[2],
            gltfMaterial.pbrMetallicRoughness.baseColorFactor[3]
        ));
    }

    material->SetMetallicFactor(gltfMaterial.pbrMetallicRoughness.metallicFactor);
    material->SetRoughnessFactor(gltfMaterial.pbrMetallicRoughness.roughnessFactor);

    if (gltfMaterial.pbrMetallicRoughness.baseColorTexture.index >= 0) {
        auto texture = LoadTextureFromGLTF(gltfMaterial.pbrMetallicRoughness.baseColorTexture.index);
        if (texture) {
            material->SetBaseColorTexture(texture);
        }
    }

    if (gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
        auto texture = LoadTextureFromGLTF(gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index);
        if (texture) {
            material->SetMetallicRoughnessTexture(texture);
        }
    }

    if (gltfMaterial.normalTexture.index >= 0) {
        auto texture = LoadTextureFromGLTF(gltfMaterial.normalTexture.index);
        if (texture) {
            material->SetNormalTexture(texture);
            material->SetFloat("u_normalScale", gltfMaterial.normalTexture.scale);
        }
    }

    if (gltfMaterial.emissiveFactor.size() == 3) {
        material->SetEmissiveFactor(glm::vec3(
            gltfMaterial.emissiveFactor[0],
            gltfMaterial.emissiveFactor[1],
            gltfMaterial.emissiveFactor[2]
        ));
    }

    if (gltfMaterial.emissiveTexture.index >= 0) {
        auto texture = LoadTextureFromGLTF(gltfMaterial.emissiveTexture.index);
        if (texture) {
            material->SetEmissiveTexture(texture);
        }
    }

    if (gltfMaterial.occlusionTexture.index >= 0) {
        auto texture = LoadTextureFromGLTF(gltfMaterial.occlusionTexture.index);
        if (texture) {
            material->SetTexture("u_occlusionTexture", texture);
            material->SetFloat("u_occlusionStrength", gltfMaterial.occlusionTexture.strength);
        }
    }

    return material;
}

std::shared_ptr<Texture> Model::LoadTextureFromGLTF(int textureIndex) {
    if (textureIndex < 0 || textureIndex >= m_gltfModel.textures.size()) {
        return nullptr;
    }

    auto cacheIt = m_textureCache.find(textureIndex);
    if (cacheIt != m_textureCache.end()) {
        return cacheIt->second;
    }

    const auto& gltfTexture = m_gltfModel.textures[textureIndex];
    if (gltfTexture.source < 0 || gltfTexture.source >= m_gltfModel.images.size()) {
        return nullptr;
    }

    const auto& image = m_gltfModel.images[gltfTexture.source];
    
    try {
        std::shared_ptr<Texture> texture;
        
        if (!image.image.empty()) {
            texture = CreateTextureFromData(
                image.width, image.height, 1,
                TextureType::Tex2D,
                image.component == 4 ? TextureFormat::RGBA : TextureFormat::RGB,
                image.component == 4 ? TextureInternalFormat::RGBA8 : TextureInternalFormat::RGB8,
                image.image.data()
            );
        } else if (!image.uri.empty()) {
            std::filesystem::path modelPath(m_filepath);
            std::string texturePath = (modelPath.parent_path() / image.uri).string();
            texture = CreateTextureFromFile(texturePath);
        }

        if (texture) {
            if (gltfTexture.sampler >= 0 && gltfTexture.sampler < m_gltfModel.samplers.size()) {
                const auto& sampler = m_gltfModel.samplers[gltfTexture.sampler];
                if (sampler.minFilter == TINYGLTF_TEXTURE_FILTER_NEAREST) {
                    texture->minFilter = TextureFilter::Nearest;
                } else if (sampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR) {
                    texture->minFilter = TextureFilter::Linear;
                }
                
                if (sampler.magFilter == TINYGLTF_TEXTURE_FILTER_NEAREST) {
                    texture->magFilter = TextureFilter::Nearest;
                } else if (sampler.magFilter == TINYGLTF_TEXTURE_FILTER_LINEAR) {
                    texture->magFilter = TextureFilter::Linear;
                }

                if (sampler.wrapS == TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE) {
                    texture->wrapS = TextureWrap::ClampToEdge;
                } else if (sampler.wrapS == TINYGLTF_TEXTURE_WRAP_REPEAT) {
                    texture->wrapS = TextureWrap::Repeat;
                }
                
                if (sampler.wrapT == TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE) {
                    texture->wrapT = TextureWrap::ClampToEdge;
                } else if (sampler.wrapT == TINYGLTF_TEXTURE_WRAP_REPEAT) {
                    texture->wrapT = TextureWrap::Repeat;
                }
                
                texture->applyParameters();
            }
            
            m_textureCache[textureIndex] = texture;
            m_textures.push_back(texture);
        }
        
        return texture;
    } catch (const std::exception& e) {
        spdlog::error("Failed to load texture {}: {}", image.name.empty() ? "unnamed" : image.name, e.what());
        return nullptr;
    }
}

glm::mat4 Model::GetNodeTransform(const tinygltf::Node& node) {
    glm::mat4 transform(1.0f);
    
    if (node.matrix.size() == 16) {
        transform = glm::make_mat4(node.matrix.data());
    } else {
        if (node.translation.size() == 3) {
            transform = glm::translate(transform, glm::vec3(
                node.translation[0], node.translation[1], node.translation[2]
            ));
        }
        
        if (node.rotation.size() == 4) {
            glm::quat rotation(
                node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]
            );
            transform *= glm::mat4_cast(rotation);
        }
        
        if (node.scale.size() == 3) {
            transform = glm::scale(transform, glm::vec3(
                node.scale[0], node.scale[1], node.scale[2]
            ));
        }
    }
    
    return transform;
}

void Model::ExtractVertexData(const tinygltf::Primitive& primitive, 
                              std::vector<Vertex3D>& vertices, 
                              std::vector<uint32_t>& indices) {
    
    size_t vertexStart = vertices.size();

    size_t vertexCount = 0;
    auto posIt = primitive.attributes.find("POSITION");
    if (posIt != primitive.attributes.end()) {
        const auto& accessor = m_gltfModel.accessors[posIt->second];
        vertexCount = accessor.count;
        vertices.resize(vertexStart + vertexCount);
    }
    
    if (posIt != primitive.attributes.end()) {
        const auto& accessor = m_gltfModel.accessors[posIt->second];
        const auto& bufferView = m_gltfModel.bufferViews[accessor.bufferView];
        const auto& buffer = m_gltfModel.buffers[bufferView.buffer];
        
        const float* positions = reinterpret_cast<const float*>(
            buffer.data.data() + bufferView.byteOffset + accessor.byteOffset
        );
        
        for (size_t i = 0; i < vertexCount; ++i) {
            vertices[vertexStart + i].position = glm::vec3(
                positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]
            );
            UpdateBoundingBox(vertices[vertexStart + i].position);
        }
    }
    
    auto normIt = primitive.attributes.find("NORMAL");
    if (normIt != primitive.attributes.end()) {
        const auto& accessor = m_gltfModel.accessors[normIt->second];
        const auto& bufferView = m_gltfModel.bufferViews[accessor.bufferView];
        const auto& buffer = m_gltfModel.buffers[bufferView.buffer];
        
        const float* normals = reinterpret_cast<const float*>(
            buffer.data.data() + bufferView.byteOffset + accessor.byteOffset
        );
        
        for (size_t i = 0; i < vertexCount; ++i) {
            vertices[vertexStart + i].normal = glm::vec3(
                normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]
            );
        }
    }

    auto texIt = primitive.attributes.find("TEXCOORD_0");
    if (texIt != primitive.attributes.end()) {
        const auto& accessor = m_gltfModel.accessors[texIt->second];
        const auto& bufferView = m_gltfModel.bufferViews[accessor.bufferView];
        const auto& buffer = m_gltfModel.buffers[bufferView.buffer];
        
        const float* texCoords = reinterpret_cast<const float*>(
            buffer.data.data() + bufferView.byteOffset + accessor.byteOffset
        );
        
        for (size_t i = 0; i < vertexCount; ++i) {
            vertices[vertexStart + i].texCoords = glm::vec2(
                texCoords[i * 2], texCoords[i * 2 + 1]
            );
        }
    }
    
    if (primitive.indices >= 0) {
        const auto& accessor = m_gltfModel.accessors[primitive.indices];
        const auto& bufferView = m_gltfModel.bufferViews[accessor.bufferView];
        const auto& buffer = m_gltfModel.buffers[bufferView.buffer];
        
        const uint8_t* data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
        
        for (size_t i = 0; i < accessor.count; ++i) {
            uint32_t index = 0;
            
            if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                index = static_cast<uint32_t>(data[i]);
            } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                const uint16_t* indices16 = reinterpret_cast<const uint16_t*>(data);
                index = static_cast<uint32_t>(indices16[i]);
            } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                const uint32_t* indices32 = reinterpret_cast<const uint32_t*>(data);
                index = indices32[i];
            }
            
            indices.push_back(static_cast<uint32_t>(vertexStart) + index);
        }
    }
}

void Model::UpdateBoundingBox(const glm::vec3& point) {
    m_boundingBoxMin = glm::min(m_boundingBoxMin, point);
    m_boundingBoxMax = glm::max(m_boundingBoxMax, point);
}

void Model::CalculateBoundingBox() {
    m_boundingBoxMin = glm::vec3(FLT_MAX);
    m_boundingBoxMax = glm::vec3(-FLT_MAX);
    
    for (const auto& mesh : m_meshes) {
        for (const auto& vertex : mesh->GetVertices()) {
            UpdateBoundingBox(vertex.position);
        }
    }
    
    spdlog::info("Model bounding box: min({:.2f}, {:.2f}, {:.2f}), max({:.2f}, {:.2f}, {:.2f})",
                 m_boundingBoxMin.x, m_boundingBoxMin.y, m_boundingBoxMin.z,
                 m_boundingBoxMax.x, m_boundingBoxMax.y, m_boundingBoxMax.z);
}

void Model::Draw(const glm::mat4& modelMatrix) const {
    DrawNode(*m_rootNode, modelMatrix);
}

void Model::DrawNode(const ModelNode& node, const glm::mat4& parentTransform) const {
    glm::mat4 nodeTransform = parentTransform * node.transform;
    
    for (int meshIndex : node.meshIndices) {
        if (meshIndex >= 0 && meshIndex < m_meshes.size()) {
            const auto& mesh = m_meshes[meshIndex];
            
            std::shared_ptr<Material> material = nullptr;
            if (!mesh->m_subMeshes.empty() && mesh->m_subMeshes[0].material) {
                material = mesh->m_subMeshes[0].material;
            } else {
                material = mesh->GetDefaultMaterial();
            }
            
            if (material && material->GetShader()) {
                material->GetShader()->SetUniformMat4("u_modelMatrix", nodeTransform);
            }
            
            mesh->Draw();
        }
    }
    
    for (const auto& child : node.children) {
        DrawNode(*child, nodeTransform);
    }
}

void Model::SetShaderForAllMaterials(std::shared_ptr<ShaderProgram> shader) {
    for (auto& material : m_materials) {
        material->SetShader(shader);
    }
    
    for (auto& mesh : m_meshes) {
        if (auto defaultMat = mesh->GetDefaultMaterial()) {
            defaultMat->SetShader(shader);
        }
    }
}