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

    PerlinTex = CreateEmptyTexture(
        width, height, 1,
        TextureType::Tex2D,
        TextureInternalFormat::RGBA8, 
        0,                          
        TextureFilter::Linear,        
        TextureFilter::Linear,       
        TextureWrap::Repeat,   
        TextureWrap::Repeat,      
        TextureWrap::Repeat
    );

    auto compPtr = CreateShader(ShaderStage::Compute, "Shader/Perlin.comp");
    PerlinShader = CreateShaderProgram(*compPtr);

    PerlinShader->useShaderProgram();
    PerlinShader->SetUniform1f("uScale", 16.0f);      
    PerlinShader->SetUniform1i("uOctaves", 4);     
    PerlinShader->SetUniform1f("uPersistence", 0.5f); 
    PerlinShader->SetUniform1f("uLacunarity", 2.0f);   

    PerlinTex->BindTextureForImageAccess(1, PerlinTex->id, TextureAccess::Write, TextureInternalFormat::RGBA8);
    
    // Initial compute dispatch
    PerlinShader->groupsX = (width + 15) / 16;
    PerlinShader->groupsY = (height + 15) / 16;
    PerlinShader->groupsZ = 1;
    PerlinShader->DispatchCompute();
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

inline void updatePerlinNoise() {
    PerlinShader->useShaderProgram();
    float currentTime = SDL_GetTicks() / 1000.0f;
    
    PerlinShader->SetUniform1f("uTime", currentTime * 0.3f);
    
    PerlinShader->SetUniform1f("uScale", 16.0f);    
    PerlinShader->SetUniform1i("uOctaves", 4);          
    PerlinShader->SetUniform1f("uPersistence", 0.5f);   
    PerlinShader->SetUniform1f("uLacunarity", 2.0f);   
    
    PerlinShader->DispatchCompute();
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

inline void drawPerlin() {
    updatePerlinNoise();
    
    Perlinvao->bind();
    PerlinCubeShader->useShaderProgram();
    
    float currentTime = SDL_GetTicks() / 1000.0f;
    PerlinCubeShader->SetUniform1f("uTime", currentTime);
    PerlinCubeShader->SetUniform1f("uScrollSpeed", 0.1f);  
    PerlinCubeShader->SetUniform1f("uSpacing", 1.0f);        
    PerlinCubeShader->SetUniform1f("uCubeSize", 0.8f);       
    PerlinCubeShader->SetUniform1f("uHeightScale", 8.0f);   
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

inline void drawPerlinSmooth() {
    updatePerlinNoise();
    
    Perlinvao->bind();
    PerlinCubeShader->useShaderProgram();
    
    float currentTime = SDL_GetTicks() / 1000.0f;
    PerlinCubeShader->SetUniform1f("uTime", currentTime);
    PerlinCubeShader->SetUniform1f("uScrollSpeed", 0.05f);    
    PerlinCubeShader->SetUniform1f("uSpacing", 2.f);    
    PerlinCubeShader->SetUniform1f("uCubeSize", .9f);     
    PerlinCubeShader->SetUniform1f("uHeightScale", 5.0f);
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


inline void drawPerlinDebugSeamless() {
    updatePerlinNoise();
    
    Perlinvao->bind();
    PerlinCubeShader->useShaderProgram();
    
    float currentTime = SDL_GetTicks() / 1000.0f;
    PerlinCubeShader->SetUniform1f("uTime", currentTime);
    PerlinCubeShader->SetUniform1f("uScrollSpeed", 0.2f);
    PerlinCubeShader->SetUniform1f("uSpacing", 2.0f);
    PerlinCubeShader->SetUniform1f("uCubeSize", 1.5f);   
    PerlinCubeShader->SetUniform1f("uHeightScale", 10.0f);  
    PerlinCubeShader->SetUniform1i("uTextureSize", 64); 
    PerlinCubeShader->SetUniform1i("uTexture", 0);
    
    PerlinTex->BindTextureForSampling(0);
    
    OpenGLSettings settings;
    settings.depthTest = true;
    settings.cullFace = false;
    settings.blend = false;
    settings.Apply();

    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 64 * 64);
}
