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

static float perlinCubeVertices[] = {
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

struct PerlinVertex {
    glm::vec3 position;
};

inline std::unique_ptr<ShaderProgram> PerlinShader;
inline std::unique_ptr<ShaderProgram> PerlinCubeShader;
inline std::unique_ptr<VertexArray> Perlinvao;
inline std::unique_ptr<Buffer> Perlinvbo;
inline std::unique_ptr<Texture> PerlinTex;

int width = 256;
int height = 256;

inline void initPerlin() {
    VertexDescription PerlinDesc;
    PerlinDesc.stride = sizeof(PerlinVertex);
    PerlinDesc.attributes = {{0, 3, GL_FLOAT, offsetof(PerlinVertex, position), false}};

    auto vertPtr = CreateShader(ShaderStage::Vertex, "Shader/Perlin.vert.glsl");
    auto fragPtr = CreateShader(ShaderStage::Fragment, "Shader/Perlin.frag.glsl");
    PerlinCubeShader = CreateShaderProgram({vertPtr.get(), fragPtr.get()});

    Perlinvao = CreateVertexArray();
    Perlinvao->desc = PerlinDesc;
    Perlinvbo = CreateBuffer(BufferType::Vertex, sizeof(perlinCubeVertices), perlinCubeVertices, BufferUsage::Static, 0);
    Perlinvao->ApplyLayout(Perlinvbo->id);

    // CRITICAL: Create texture with PROPER settings for seamless tiling
    PerlinTex = CreateEmptyTexture(
        width, height, 1,
        TextureType::Tex2D,
        TextureInternalFormat::RGBA8,  // Changed from RGB8 to RGBA8 to match compute shader
        0,                             // Unit 0
        TextureFilter::Linear,         // Linear filtering for interpolation
        TextureFilter::Linear,         // Linear filtering for interpolation
        TextureWrap::Repeat,          // REPEAT for seamless wrapping
        TextureWrap::Repeat,          // REPEAT for seamless wrapping
        TextureWrap::Repeat
    );

    // Create compute shader with seamless noise generation
    auto compPtr = CreateShader(ShaderStage::Compute, "Shader/Perlin.comp");
    PerlinShader = CreateShaderProgram(*compPtr);

    // Set compute shader uniforms for seamless generation
    PerlinShader->useShaderProgram();
    PerlinShader->SetUniform1f("uScale", 16.0f);         // Scale must match repeat period for seamless
    PerlinShader->SetUniform1i("uOctaves", 4);           // Balanced detail
    PerlinShader->SetUniform1f("uPersistence", 0.5f);    // Moderate persistence
    PerlinShader->SetUniform1f("uLacunarity", 2.0f);     // Standard lacunarity

    // Bind texture to image unit for compute shader writing
    PerlinTex->BindTextureForImageAccess(1, PerlinTex->id, TextureAccess::Write, TextureInternalFormat::RGBA8);
    
    // Initial compute dispatch
    PerlinShader->groupsX = (width + 15) / 16;
    PerlinShader->groupsY = (height + 15) / 16;
    PerlinShader->groupsZ = 1;
    PerlinShader->DispatchCompute();
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

// Updated noise generation with seamless parameters
inline void updatePerlinNoise() {
    PerlinShader->useShaderProgram();
    float currentTime = SDL_GetTicks() / 1000.0f;
    
    // Set time uniform for animation
    PerlinShader->SetUniform1f("uTime", currentTime * 0.3f); // Moderate time progression
    
    // CRITICAL: These parameters must create seamless noise
    PerlinShader->SetUniform1f("uScale", 16.0f);        // Must match repeat period
    PerlinShader->SetUniform1i("uOctaves", 4);          // Good balance
    PerlinShader->SetUniform1f("uPersistence", 0.5f);   // Moderate
    PerlinShader->SetUniform1f("uLacunarity", 2.0f);    // Standard
    
    // Dispatch compute shader
    PerlinShader->DispatchCompute();
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

inline void drawPerlin() {
    // Update noise texture first
    updatePerlinNoise();
    
    // Setup cube rendering
    Perlinvao->bind();
    PerlinCubeShader->useShaderProgram();
    
    // Set cube shader uniforms
    float currentTime = SDL_GetTicks() / 1000.0f;
    PerlinCubeShader->SetUniform1f("uTime", currentTime);
    PerlinCubeShader->SetUniform1f("uScrollSpeed", 0.1f);     // Moderate scroll speed
    PerlinCubeShader->SetUniform1f("uSpacing", 1.0f);        // Distance between cubes
    PerlinCubeShader->SetUniform1f("uCubeSize", 0.8f);       // Size of each cube  
    PerlinCubeShader->SetUniform1f("uHeightScale", 8.0f);    // Height multiplier
    PerlinCubeShader->SetUniform1i("uTextureSize", width);   // Use width variable
    PerlinCubeShader->SetUniform1i("uTexture", 0);           // Texture unit 0
    
    // Bind texture for sampling in vertex shader
    PerlinTex->BindTextureForSampling(0);
    
    // Apply OpenGL settings for 3D rendering
    OpenGLSettings settings;
    settings.depthTest = true;
    settings.cullFace = true;
    settings.cullMode = GL_BACK;
    settings.blend = false;  // Disable blending for opaque cubes
    settings.Apply();
    
    // Draw all instances
    int totalInstances = width * height; // 256 * 256 = 65,536
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, totalInstances);
}

// Smooth version with optimal parameters for seamless scrolling
inline void drawPerlinSmooth() {
    updatePerlinNoise();
    
    Perlinvao->bind();
    PerlinCubeShader->useShaderProgram();
    
    float currentTime = SDL_GetTicks() / 1000.0f;
    PerlinCubeShader->SetUniform1f("uTime", currentTime);
    PerlinCubeShader->SetUniform1f("uScrollSpeed", 0.05f);    // Slow, smooth scrolling
    PerlinCubeShader->SetUniform1f("uSpacing", 2.f);        // Tight spacing for continuous look
    PerlinCubeShader->SetUniform1f("uCubeSize", .9f);       // Smaller cubes for smooth appearance
    PerlinCubeShader->SetUniform1f("uHeightScale", 5.0f);    // Moderate height variation
    PerlinCubeShader->SetUniform1i("uTextureSize", width);
    PerlinCubeShader->SetUniform1i("uTexture", 0);
    
    PerlinTex->BindTextureForSampling(0);
    
    OpenGLSettings settings;
    settings.depthTest = true;
    settings.cullFace = true;
    settings.cullMode = GL_BACK;
    settings.blend = false;
    settings.Apply();
    
    int totalInstances = width * height;
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, totalInstances);
}

// Debug version to test seamless tiling
inline void drawPerlinDebugSeamless() {
    updatePerlinNoise();
    
    Perlinvao->bind();
    PerlinCubeShader->useShaderProgram();
    
    float currentTime = SDL_GetTicks() / 1000.0f;
    PerlinCubeShader->SetUniform1f("uTime", currentTime);
    PerlinCubeShader->SetUniform1f("uScrollSpeed", 0.2f);     // Fast scroll to see seams if they exist
    PerlinCubeShader->SetUniform1f("uSpacing", 2.0f);        // Wide spacing for easy visibility
    PerlinCubeShader->SetUniform1f("uCubeSize", 1.5f);       // Large cubes
    PerlinCubeShader->SetUniform1f("uHeightScale", 10.0f);   // Exaggerated height
    PerlinCubeShader->SetUniform1i("uTextureSize", 64);      // Smaller grid for testing
    PerlinCubeShader->SetUniform1i("uTexture", 0);
    
    PerlinTex->BindTextureForSampling(0);
    
    OpenGLSettings settings;
    settings.depthTest = true;
    settings.cullFace = false;  // Disable culling for debugging
    settings.blend = false;
    settings.Apply();
    
    // Draw only 64x64 = 4,096 cubes for debugging
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 64 * 64);
}
