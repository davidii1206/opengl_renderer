// ----------------------------------------
// Textures
// ----------------------------------------
enum class TextureType { Tex2D, Tex3D, CubeMap };
enum class TextureFormat { RGBA8, RGBA16F, Depth24Stencil8 };

struct Texture {
    uint32_t id;
    int width, height, depth;
    TextureType type;
    TextureFormat format;
};

Texture glrCreateTexture2D(int width, int height, TextureFormat fmt, const void* data = nullptr);
void    glrUpdateTexture2D(Texture& tex, int x, int y, int w, int h, const void* data);
void    glrDestroyTexture(Texture& tex);

// ----------------------------------------
// Framebuffers / Render Targets
// ----------------------------------------
struct Framebuffer {
    uint32_t id;
    Texture color;
    Texture depth;
};

Framebuffer glrCreateFramebuffer(Texture color, Texture depth = {});
void        glrBindFramebuffer(const Framebuffer& fb);
void        glrDestroyFramebuffer(Framebuffer& fb);


// --------------------------
// Raw Data (bytes)
void glrUploadRawData(Buffer& buf, const void* data, size_t size, size_t offset = 0);
void glrBindRawBufferToShader(const Shader& shader, const std::string& name, const Buffer& buf, int binding);

#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <memory>
#include "gpu_data_layer.h" // the low-level API from before

// -------------------------
// Shader / Material
// -------------------------
struct ShaderProgram {
    gpu::Shader program;
    std::vector<ShaderStage> stages; // vertex, fragment, etc
};

ShaderProgram rLoadShaderProgram(const std::vector<std::pair<std::string, ShaderStage>>& paths);

struct Material {
    ShaderProgram shader;
    std::vector<Texture> textures;        // bound by name
    glm::vec4 color = glm::vec4(1.0f);   // default diffuse
    bool depthTest = true;
    bool blend = false;
    bool cull = true;

    void setTexture(const std::string& name, const Texture& tex);
    void setUniform(const std::string& name, float v);
    void setUniform(const std::string& name, const glm::vec3& v);
    void setUniform(const std::string& name, const glm::vec4& v);
    void setUniform(const std::string& name, const glm::mat4& v);
};

// -------------------------
// Mesh
// -------------------------
struct Mesh {
    gpu::VertexArray vao;
    gpu::Buffer vbo;
    gpu::Buffer ibo;
    int indexCount;
    VertexLayout layout;
    std::shared_ptr<Material> material;

    void draw(const glm::mat4& modelMatrix, const Camera& cam);
};

// -------------------------
// Model (multiple meshes)
// -------------------------
struct Model {
    std::vector<Mesh> meshes;

    void draw(const glm::mat4& modelMatrix, const Camera& cam) {
        for (auto& mesh : meshes)
            mesh.draw(modelMatrix, cam);
    }
};

// -------------------------
// Utilities
// -------------------------
VertexLayout rDefineVertexLayout(const std::vector<VertexAttrib>& attribs);
Mesh rCreateMesh(const void* vertexData, size_t vertexSize, size_t vertexCount,
                 const void* indexData, size_t indexCount,
                 const VertexLayout& layout,
                 std::shared_ptr<Material> mat);
Model rLoadModel(const std::string& filepath); // uses Assimp internally

#pragma once
#include <glm/glm.hpp>

enum class CameraType { Perspective, Orthographic };



// ------------------------
// Editor Camera
// ------------------------
struct EditorCamera : public Camera {
    float yaw   = -90.0f; // horizontal angle
    float pitch = 0.0f;   // vertical angle
    float distance = 10.0f; // orbit distance
    bool orbiting = false;

    // Input
    void onMouseDrag(float dx, float dy, bool altPressed = false);
    void onMouseWheel(float delta);

    // Update matrices after movement
    void update();
};

#pragma once
#include <glm/glm.hpp>
#include "camera.h"
#include "gpu_data_layer.h"
#include "mesh_material_api.h"

// ------------------------
// Render Target (Framebuffer wrapper)
struct RenderTarget {
    gpu::Framebuffer fb;
    gpu::Texture color;
    gpu::Texture depth;

    int width, height;

    void bind();
    void unbind();
};

// ------------------------
// Framebuffer + Camera Rendering
// ------------------------
void rBeginRenderPass(const RenderTarget& target, const glm::vec4& clearColor = {0,0,0,1});
void rEndRenderPass();

void rDrawMesh(const Mesh& mesh, const glm::mat4& modelMatrix, const Camera& cam);

// ------------------------
// Screen Utilities
// ------------------------
glm::vec3 rScreenToWorld(const Camera& cam, const glm::vec2& screenPos, float depth = 1.0f);
glm::vec3 rScreenToRay(const Camera& cam, const glm::vec2& screenPos);

// ------------------------
// Editor Viewport Helpers
// ------------------------
struct EditorViewport {
    RenderTarget target;
    EditorCamera camera;

    // Bind viewport for rendering
    void begin();
    void end();

    // Generate picking ray in world space from mouse
    glm::vec3 getPickingRay(const glm::vec2& mousePos);
};

#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "gpu_data_layer.h"
#include "shader_material_api.h"

// ------------------------
// Light Types
// ------------------------
enum class LightType { Directional, Point, Spot };

// Light data structure (CPU side)
struct Light {
    LightType type;

    glm::vec3 position   = {0,0,0}; // world space
    glm::vec3 direction  = {0,-1,0};
    glm::vec3 color      = {1,1,1};
    float intensity      = 1.0f;

    // Point/Spot lights
    float range          = 10.0f;
    
    // Spot lights
    float innerAngle     = 30.0f; // degrees
    float outerAngle     = 45.0f;

    // Optional shadow map (engine-defined texture)
    std::shared_ptr<Texture> shadowMap = nullptr;
};

// ------------------------
// Light Manager
// ------------------------
struct LightManager {
    std::vector<std::shared_ptr<Light>> lights;

    // Add/remove lights
    std::shared_ptr<Light> addLight(LightType type);
    void removeLight(const std::shared_ptr<Light>& light);

    // Send to shader (as uniform array or SSBO)
    void bindLights(const ShaderProgram& shader, int binding = 0);
};

// ------------------------
// Helper functions for clients
// ------------------------
glm::mat4 getLightViewMatrix(const Light& light);   // for shadow mapping
glm::mat4 getLightProjMatrix(const Light& light);   // perspective or orthographic

struct Material {
    ShaderProgram shader;
    std::vector<Texture> textures;    
    glm::vec4 color = glm::vec4(1.0f);

    bool depthTest = true;
    bool blend = false;
    bool cull = true;

    // NEW: lighting control
    bool affectedByLights = true;

    // Uniform helpers
    void setUniform(const std::string& name, float v);
    void setUniform(const std::string& name, const glm::vec3& v);
    void setUniform(const std::string& name, const glm::vec4& v);
    void setUniform(const std::string& name, const glm::mat4& v);

    void setTexture(const std::string& name, const Texture& tex);

    // Bind material for a mesh
    void apply(const LightManager* lights = nullptr);
};

#pragma once
#include <vector>
#include <memory>
#include "gpu_data_layer.h"
#include "shader_material_api.h"
#include "render_target_api.h"

// ------------------------
// Post-processing pass
// ------------------------
struct PostProcessPass {
    ShaderProgram shader;               // shader to process input
    std::vector<RenderTarget*> inputs;  // input render targets (color, depth, normals, etc.)
    RenderTarget* output;               // output render target (ping-pong)

    bool enabled = true;

    void apply(); // binds inputs, output, draws fullscreen quad
};

// ------------------------
// Post-processing chain
// ------------------------
struct PostProcessChain {
    std::vector<std::shared_ptr<PostProcessPass>> passes;

    // Apply all enabled passes
    void execute();

    // Convenience
    void addPass(std::shared_ptr<PostProcessPass> pass);
    void removePass(std::shared_ptr<PostProcessPass> pass);
};

// ------------------------
// Fullscreen quad helper
// ------------------------
void rDrawFullscreenQuad(const ShaderProgram& shader, const std::vector<RenderTarget*>& inputs, RenderTarget* output);

#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "mesh_material_api.h"
#include "camera.h"
#include "light_api.h"

// ------------------------
// Instancing
// ------------------------
struct InstanceData {
    glm::mat4 model;
    glm::vec4 color;
    // Optional: custom per-instance data
};

struct InstancedMesh {
    Mesh mesh;                     // Base mesh
    std::vector<InstanceData> instances;
    gpu::Buffer instanceBuffer;    // VBO for instance attributes

    void uploadInstances();        // Upload to GPU
    void drawInstanced(const Camera& cam, const LightManager* lights = nullptr);
};

// ------------------------
// Culling
// ------------------------
struct Frustum {
    glm::vec4 planes[6]; // left, right, top, bottom, near, far
    void update(const Camera& cam);
    bool isBoxVisible(const glm::vec3& min, const glm::vec3& max) const;
};

void performCulling(const std::vector<Mesh>& meshes, const Frustum& frustum, std::vector<Mesh*>& outVisible);

// ------------------------
// Batching
// ------------------------
struct Batch {
    std::shared_ptr<Material> material;
    std::vector<Mesh*> meshes;

    void drawBatch(const Camera& cam, const LightManager* lights = nullptr);
};

// ------------------------
// Multi-threading helpers
// ------------------------
void parallelFor(size_t start, size_t end, std::function<void(size_t)> func);

// ------------------------
// GPU Profiling
// ------------------------
struct GPUProfiler {
    void startQuery(const std::string& name);
    void endQuery(const std::string& name);
    float getElapsedMS(const std::string& name);
    void reset();
};

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float size;
    float rotation;
    float life; // remaining life
};

struct ParticleEmitter {
    std::vector<Particle> particles;
    gpu::Buffer instanceBuffer; // upload per-particle attributes

    std::shared_ptr<Material> material;

    glm::vec3 position;
    glm::vec3 velocityMin, velocityMax;
    glm::vec4 colorStart, colorEnd;
    float particleSizeMin, particleSizeMax;
    float lifeMin, lifeMax;
    float emissionRate; // particles/sec
    bool looping = true;

    void update(float dt);      // CPU update
    void upload();              // upload instance data to GPU
    void draw(const Camera& cam, const LightManager* lights = nullptr);
};

struct ParticleSystem {
    std::vector<std::shared_ptr<ParticleEmitter>> emitters;

    void update(float dt);
    void draw(const Camera& cam, const LightManager* lights = nullptr);
};