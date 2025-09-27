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

static std::vector<std::string> faces = {
    "Textures/Cubemap/right.jpg",
    "Textures/Cubemap/left.jpg",
    "Textures/Cubemap/top.jpg",
    "Textures/Cubemap/bottom.jpg",
    "Textures/Cubemap/front.jpg",
    "Textures/Cubemap/back.jpg"
};

struct CubemapVertex {
    glm::vec3 position;
};

inline std::unique_ptr<VertexArray> vaoCube;
inline std::unique_ptr<Buffer> vboCube;
inline std::unique_ptr<Texture> cubemapTexture;
inline std::unique_ptr<ShaderProgram> cubemapShader;

inline void initCube() {

    
    std::cout << "Initializing cube...\n";
    VertexDescription cubeDesc;
    cubeDesc.stride = sizeof(CubemapVertex);
    cubeDesc.attributes = {{0, 3, GL_FLOAT, offsetof(CubemapVertex, position), false}};

    vaoCube = CreateVertexArray();
    vaoCube->desc = cubeDesc;

    vboCube = CreateBuffer(BufferType::Vertex, sizeof(cubeVertices), cubeVertices, BufferUsage::Static, 0);
    vaoCube->ApplyLayout(vboCube->id);

    cubemapTexture = CreateTextureFromFile(
        "",                                 
        &faces,                              
        TextureType::CubeMap,              
        TextureFormat::RGB,                   
        TextureInternalFormat::RGB8,       
        1,                                  
        TextureFilter::Linear,            
        TextureFilter::Linear,               
        TextureWrap::ClampToEdge,          
        TextureWrap::ClampToEdge,          
        TextureWrap::ClampToEdge
    );

    auto vertPtr = CreateShader(ShaderStage::Vertex, "Shader/cubemap.vert.glsl");
    auto fragPtr = CreateShader(ShaderStage::Fragment, "Shader/cubemap.frag.glsl");
    cubemapShader = CreateShaderProgram({vertPtr.get(), fragPtr.get()});

}

inline void drawCubemap() {

    GLint prevDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFunc);

    glDepthFunc(GL_LEQUAL);

    vaoCube->bind();
    cubemapShader->useShaderProgram();

    cubemapShader->SetUniform1i("uCubemap", 0);
    cubemapTexture->BindTextureForSampling(0);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthFunc(prevDepthFunc);
}