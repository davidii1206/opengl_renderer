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

int width = 256;
int height = 256;
GLuint texID;

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

    // Create texture with settings optimized for seamless tiling
    glCreateTextures(GL_TEXTURE_2D, 1, &texID);
    glTextureStorage2D(texID, 1, GL_RGBA8, width, height);
    
    // CRITICAL: These settings are essential for seamless tiling
    glTextureParameteri(texID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(texID, GL_TEXTURE_WRAP_S, GL_REPEAT);     // Must be REPEAT for seamless
    glTextureParameteri(texID, GL_TEXTURE_WRAP_T, GL_REPEAT);     // Must be REPEAT for seamless
    
    // Optional: Add some border color just in case (should not be visible with seamless noise)
    float borderColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glTextureParameterfv(texID, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Create compute shader (use the new seamless version!)
    auto compPtr = CreateShader(ShaderStage::Compute, "Shader/Perlin.comp"); // New shader file!
    PerlinShader = CreateShaderProgram(*compPtr);

    // Set compute shader uniforms for seamless generation
    PerlinShader->useShaderProgram();
    PerlinShader->SetUniform1f("uScale", 5.0f);         // Lower scale for smoother patterns
    PerlinShader->SetUniform1i("uOctaves", 1);           // Good balance of detail
    PerlinShader->SetUniform1f("uPersistence", 0.6f);    // Moderate persistence
    PerlinShader->SetUniform1f("uLacunarity", 2.0f);     // Standard lacunarity

    // Bind texture to image unit
    glBindImageTexture(1, texID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    
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
    PerlinShader->SetUniform1f("uTime", currentTime * 0.5f); // Slower time progression
    
    // These parameters are critical for seamless generation
    PerlinShader->SetUniform1f("uScale", 5.0f);        // Must match the repeat period
    PerlinShader->SetUniform1i("uOctaves", 1);
    PerlinShader->SetUniform1f("uPersistence", 0.6f);
    PerlinShader->SetUniform1f("uLacunarity", 2.0f);
    
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
    PerlinCubeShader->SetUniform1f("uScrollSpeed", 15.f);
    PerlinCubeShader->SetUniform1f("uSpacing", 2.f);      // Distance between cubes
    PerlinCubeShader->SetUniform1f("uCubeSize", 1.f);     // Size of each cube  
    PerlinCubeShader->SetUniform1f("uHeightScale", 100.0f);  // Height multiplier
    PerlinCubeShader->SetUniform1i("uTextureSize", width); // Use width variable
    PerlinCubeShader->SetUniform1i("uTexture", 0);         // Texture unit 0
    
    // Bind texture for sampling in vertex shader
    glBindTextureUnit(0, texID);
    
    // Enable depth testing for proper 3D rendering
    glEnable(GL_DEPTH_TEST);
    
    // Optional: Enable face culling for better performance
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // CRITICAL: Use instanced rendering for 65,536 cubes!
    int totalInstances = width * height; // 256 * 256 = 65,536
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, totalInstances);
    
    // Disable face culling if you enabled it
    glDisable(GL_CULL_FACE);
}

// Alternative version with performance optimizations
inline void drawPerlinOptimized() {
    // Only update noise every few frames for better performance
    static int frameCounter = 0;
    if (frameCounter % 2 == 0) { // Update every 2nd frame
        updatePerlinNoise();
    }
    frameCounter++;
    
    // Setup rendering
    Perlinvao->bind();
    PerlinCubeShader->useShaderProgram();
    
    // Set uniforms
    float currentTime = SDL_GetTicks() / 1000.0f;
    PerlinCubeShader->SetUniform1f("uTime", currentTime);
    PerlinCubeShader->SetUniform1f("uScrollSpeed", 0.2f);   // Slower scroll
    PerlinCubeShader->SetUniform1f("uSpacing", 0.5f);       // Tighter spacing
    PerlinCubeShader->SetUniform1f("uCubeSize", 0.4f);      // Smaller cubes
    PerlinCubeShader->SetUniform1f("uHeightScale", 5.0f);   // Moderate height
    PerlinCubeShader->SetUniform1i("uTextureSize", width);
    PerlinCubeShader->SetUniform1i("uTexture", 0);
    
    glBindTextureUnit(0, texID);
    
    // Rendering settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Draw all instances
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, width * height);
    
    glDisable(GL_CULL_FACE);
}

// Debug version for testing (draws fewer cubes)
inline void drawPerlinDebug() {
    updatePerlinNoise();
    
    Perlinvao->bind();
    PerlinCubeShader->useShaderProgram();
    
    float currentTime = SDL_GetTicks() / 1000.0f;
    PerlinCubeShader->SetUniform1f("uTime", currentTime);
    PerlinCubeShader->SetUniform1f("uScrollSpeed", 0.005f);
    PerlinCubeShader->SetUniform1f("uSpacing", 1.5f);      // More space for debugging
    PerlinCubeShader->SetUniform1f("uCubeSize", 1.f);     
    PerlinCubeShader->SetUniform1f("uHeightScale", 30.0f);  
    PerlinCubeShader->SetUniform1i("uTextureSize", 32);    // Smaller grid for debugging
    PerlinCubeShader->SetUniform1i("uTexture", 0);
    
    glBindTextureUnit(0, texID);
    glEnable(GL_DEPTH_TEST);
    
    // Draw only 32x32 = 1,024 cubes for debugging
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 32 * 32);
}

inline void drawPerlinSmooth() {
    updatePerlinNoise();
    
    Perlinvao->bind();
    PerlinCubeShader->useShaderProgram();
    
    float currentTime = SDL_GetTicks() / 1000.0f;
    PerlinCubeShader->SetUniform1f("uTime", currentTime);
    PerlinCubeShader->SetUniform1f("uScrollSpeed", 0.5f);
    PerlinCubeShader->SetUniform1f("uSpacing", 2.f);      // Distance between cubes
    PerlinCubeShader->SetUniform1f("uCubeSize", 1.f);     // Size of each cube  
    PerlinCubeShader->SetUniform1f("uHeightScale", 100.0f);  // Height multiplier
    PerlinCubeShader->SetUniform1i("uTextureSize", width); // Use width variable
    PerlinCubeShader->SetUniform1i("uTexture", 0);         // Texture unit 0
    
    // Bind texture unit
    glBindTextureUnit(0, texID);
    
    // Enable depth testing and face culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Draw all instances
    int totalInstances = width * height;
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, totalInstances);
    
    glDisable(GL_CULL_FACE);
}

// Alternative: Higher resolution interpolation
inline void drawPerlinHighRes() {
    updatePerlinNoise();
    
    Perlinvao->bind();
    PerlinCubeShader->useShaderProgram();
    
    float currentTime = SDL_GetTicks() / 1000.0f;
    PerlinCubeShader->SetUniform1f("uTime", currentTime);
    PerlinCubeShader->SetUniform1f("uScrollSpeed", 0.02f);     // Very slow scroll
    PerlinCubeShader->SetUniform1f("uSpacing", 0.5f);         // Very tight spacing
    PerlinCubeShader->SetUniform1f("uCubeSize", 0.45f);       // Small cubes
    PerlinCubeShader->SetUniform1f("uHeightScale", 8.0f);     // Good height variation
    PerlinCubeShader->SetUniform1i("uTextureSize", width);
    PerlinCubeShader->SetUniform1i("uTexture", 0);
    
    glBindTextureUnit(0, texID);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, width * height);
    glDisable(GL_CULL_FACE);
}

// Function to test different interpolation methods
inline void drawPerlinInterpolationTest() {
    static bool useHardwareInterpolation = true;
    
    updatePerlinNoise();
    
    Perlinvao->bind();
    PerlinCubeShader->useShaderProgram();
    
    float currentTime = SDL_GetTicks() / 1000.0f;
    PerlinCubeShader->SetUniform1f("uTime", currentTime);
    PerlinCubeShader->SetUniform1f("uScrollSpeed", 0.05f);
    PerlinCubeShader->SetUniform1f("uSpacing", 1.0f);
    PerlinCubeShader->SetUniform1f("uCubeSize", 0.8f);
    PerlinCubeShader->SetUniform1f("uHeightScale", 5.0f);
    PerlinCubeShader->SetUniform1i("uTextureSize", width);
    PerlinCubeShader->SetUniform1i("uTexture", 0);
    
    // Toggle between hardware and manual interpolation every 5 seconds
    if (int(currentTime) % 10 < 5) {
        useHardwareInterpolation = true;
        // In shader: use sampleHeightHardware()
    } else {
        useHardwareInterpolation = false;
        // In shader: use sampleHeightSmooth()
    }
    
    glBindTextureUnit(0, texID);
    glEnable(GL_DEPTH_TEST);
    
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, width * height);
}