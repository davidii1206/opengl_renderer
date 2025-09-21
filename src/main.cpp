#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <glad/glad.h>
#include <iostream>
#include <memory>

#include "Utils/fpscounter.h"
#include "Utils/CameraControls.h"
#include "Utils/Cubemap.h"
#include "Utils/Perlin.h"

#include "Renderer/OpenGL/Window/Window.h"
#include "Renderer/OpenGL/Buffer/Buffer.h"
#include "Renderer/OpenGL/Vertex/VertexArray.h"
#include "Renderer/OpenGL/Shader/Shader.h"
#include "Renderer/OpenGL/Shader/ShaderProgram.h"
#include "Renderer/OpenGL/Camera/Camera.h"
#include "Renderer/OpenGL/Texture/Texture.h"

int main(int argc, char* argv[]) {

    // Setup window parameters
    window.width  = 2560;
    window.height = 1440;
    window.title  = "My OpenGL SDL3 Window";
    window.mode   = WindowMode::Windowed;
    window.vsync  = false;
    window.hasFocus = true;

    // Create the window
    Window* winPtr = CreateWindow(window);

    // Load OpenGL functions with glad
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL context!" << std::endl;
        DestroyWindow(winPtr);
        return -1;
    }

    OpenGLSettings glSettings;
    glSettings.Apply();

    // Set viewport
    glm::ivec2 size = GetWindowSize(winPtr);

    // Camera
    Camera camera(110.0f, 16/10, 0.1f, 10000.0f);

    struct Vertex {
        glm::vec3 position;
        glm::vec2 texCoord;
    };

    VertexDescription VertexDesc;
    VertexDesc.stride = sizeof(Vertex);
    VertexDesc.attributes = {
        {0, 3, GL_FLOAT, offsetof(Vertex, position), false}, // position
        {1, 2, GL_FLOAT, offsetof(Vertex, texCoord), false}  // texCoords
    };

    float vertices[] = {
        // positions        // texCoords
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, // bottom-left
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f, // bottom-right
        0.0f,  0.5f, 0.0f,  0.5f, 1.0f  // top-center
    };

    auto vertPtr = CreateShader(ShaderStage::Vertex, "Shader/vert.glsl");
    auto fragPtr = CreateShader(ShaderStage::Fragment, "Shader/frag.glsl");
    auto programPtr = CreateShaderProgram({vertPtr.get(), fragPtr.get()});

    auto vaoPtr = CreateVertexArray();
    vaoPtr->desc = VertexDesc;
    auto vboPtr = CreateBuffer(BufferType::Vertex, sizeof(vertices), vertices, BufferUsage::Static, 0);
    vaoPtr->ApplyLayout(vboPtr->id);

    /*auto texPtr = CreateTexture(
        "Textures/Mattheo.png",        // file path
        nullptr,                       // faces = nullptr for 2D
        TextureType::Tex2D,
        TextureFormat::RGBA,
        TextureInternalFormat::RGBA8,
        0,                             // bind to texture unit 0
        TextureFilter::LinearMipmapLinear,  // min filter
        TextureFilter::Linear,              // mag filter
        TextureWrap::Repeat,                // wrap S
        TextureWrap::Repeat                 // wrap T
    );*/

    //programPtr->useShaderProgram();
    //glBindTextureUnit(0, texID);
    //GLint uni = glGetUniformLocation(programPtr->program, "uTexture");
    //glUniform1i(uni, 0);
    //programPtr->SetUniform1i("uTexture", 0);
    //texPtr->bind(0);

    // Cubemap
    initCube();

    auto UBOcamera = CreateBuffer(BufferType::Uniform, sizeof(glm::mat4) * 2, nullptr, BufferUsage::Dynamic, 0);
    camera.SetPosition(glm::vec3(-150, 0, 150));
    
    bool running = true;
    Uint64 lastFrame = SDL_GetPerformanceCounter();
    HideCursor();
    SetRelativeMouseMode(winPtr, true);

    // Perlin
    initPerlin();

    while (running) {
        Uint64 currentFrame = SDL_GetPerformanceCounter();
        float deltaTime = (currentFrame - lastFrame) / static_cast<float>(SDL_GetPerformanceFrequency());
        lastFrame = currentFrame;

        auto events = PollEvents();
        for (auto& e : events) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_WINDOW_RESIZED)
                glViewport(0, 0, s_CurrentWindow.width, s_CurrentWindow.height);
            if (e.type == SDL_EVENT_MOUSE_MOTION) {
                mouse_callback(winPtr, &camera, e.motion.x, e.motion.y);
            }
        }

        processInput(winPtr, &camera, deltaTime);

        BeginFrame(glm::vec4{0.1f, 0.1f, 0.1f, 1.f});

        UBOcamera->UpdateBuffer(glm::value_ptr(camera.GetViewMatrix()), sizeof(glm::mat4));
        UBOcamera->UpdateBuffer(glm::value_ptr(camera.GetProjectionMatrix()), sizeof(glm::mat4), sizeof(glm::mat4));

        //drawCubemap();

        vaoPtr->bind();
        programPtr->useShaderProgram(); 
        //programPtr->SetUniform1i("uTexture", 0);
        //texPtr->bind(0);
        glBindTextureUnit(0, texID);
        
        glDrawArrays(GL_TRIANGLES, 0, 3);

        drawPerlinSmooth();

        EndFrame();

        fps_counter();
    }

    // Cleanup
    DestroyWindow(winPtr);

    return 0;
}
