#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "../Renderer/OpenGL/Window/Window.h"
#include "../Renderer/OpenGL/Buffer/Buffer.h"
#include "../Renderer/OpenGL/Vertex/VertexArray.h"
#include "../Renderer/OpenGL/Shader/Shader.h"
#include "../Renderer/OpenGL/Shader/ShaderProgram.h"
#include "../Renderer/OpenGL/Camera/Camera.h"
#include "../Renderer/OpenGL/Texture/Texture.h"

// Cube vertices for a cubemap (positions only)
static float cubeVertices[] = {
    -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f
};

// Paths to cubemap faces
static std::vector<std::string> faces = {
    "Textures/Cubemap/right.jpg",
    "Textures/Cubemap/left.jpg",
    "Textures/Cubemap/top.jpg",
    "Textures/Cubemap/bottom.jpg",
    "Textures/Cubemap/front.jpg",
    "Textures/Cubemap/back.jpg"
};

// Struct for cube vertices
struct Vertex {
    glm::vec3 position;
};

// Declare resources as pointers (will be initialized later)
inline std::unique_ptr<VertexArray> vaoCube;
inline std::unique_ptr<Buffer> vboCube;
inline std::unique_ptr<Texture> cubemapTexture;
inline std::unique_ptr<ShaderProgram> cubemapShader;

// In your initCube() function, add this right after OpenGL context is ready:
inline void initCube() {

    
    std::cout << "Initializing cube...\n";
    // Vertex description
    VertexDescription cubeDesc;
    cubeDesc.stride = sizeof(Vertex);
    cubeDesc.attributes = {{0, 3, GL_FLOAT, offsetof(Vertex, position), false}};

    std::cout << "skrr\n";

    // Create VAO & VBO
    vaoCube = CreateVertexArray();
    vaoCube->desc = cubeDesc;

    std::cout << "skrr\n";

    vboCube = CreateBuffer(BufferType::Vertex, sizeof(cubeVertices), cubeVertices, BufferUsage::Static, 0);
    vaoCube->ApplyLayout(vboCube->id);

    std::cout << "skrr1\n";

    // Create cubemap texture
    // IMPORTANT: Use RGB format for JPEG images (not RGBA)
    cubemapTexture = CreateTexture(
        "",                                    // empty source for cubemap
        &faces,                               // pass faces vector
        TextureType::CubeMap,                 // cubemap type
        TextureFormat::RGB,                   // RGB format for JPEG images
        TextureInternalFormat::RGB8,          // RGB8 internal format
        1,                                    // bind to texture unit 1
        TextureFilter::Linear,                // Use linear instead of mipmap for cubemaps
        TextureFilter::Linear,                // Linear mag filter
        TextureWrap::ClampToEdge,            // Clamp to edge for S
        TextureWrap::ClampToEdge,            // Clamp to edge for T
        TextureWrap::ClampToEdge             // Clamp to edge for R
    );

    std::cout << "skrr2\n";

    // Load shaders
    auto vertPtr = CreateShader(ShaderStage::Vertex, "Shader/cubemap.vert.glsl");
    auto fragPtr = CreateShader(ShaderStage::Fragment, "Shader/cubemap.frag.glsl");
    cubemapShader = CreateShaderProgram({vertPtr.get(), fragPtr.get()});

    std::cout << "skrr\n";
}

// Draw cubemap (call inside render loop)
inline void drawCubemap() {
    // Save current depth function
    GLint prevDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFunc);
    
    // Set depth function to less-equal for skybox
    glDepthFunc(GL_LEQUAL);
    
    // Bind VAO and shader
    vaoCube->bind();
    cubemapShader->useShaderProgram();
    
    // Set the cubemap texture uniform
    cubemapShader->SetUniform1i("uCubemap", 0);
    cubemapTexture->bind(0);
    
    // Draw the skybox
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    // Restore previous depth function
    glDepthFunc(prevDepthFunc);
}