#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <glad/glad.h>
#include <iostream>
#include <memory>

#include "Utils/fpscounter.h"
#include "Utils/CameraControls.h"

#include "Renderer/OpenGL/Window/Window.h"
#include "Renderer/OpenGL/Buffer/Buffer.h"
#include "Renderer/OpenGL/Vertex/VertexArray.h"
#include "Renderer/OpenGL/Shader/Shader.h"
#include "Renderer/OpenGL/Shader/ShaderProgram.h"
#include "Renderer/OpenGL/Camera/Camera.h"

int main(int argc, char* argv[]) {

    // Setup window parameters
    window.width  = 800;
    window.height = 600;
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

    struct Vertex {
        glm::vec3 position;
    };

    VertexDescription VertexDesc;
    VertexDesc.stride = sizeof(Vertex),
    VertexDesc.attributes = {
        {0, 3, GL_FLOAT, offsetof(Vertex, position), false}
    };

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    }; 

    auto vertPtr = CreateShader(ShaderStage::Vertex, "Shader/vert.glsl");
    auto fragPtr = CreateShader(ShaderStage::Fragment, "Shader/frag.glsl");
    auto programPtr = CreateShaderProgram({vertPtr.get(), fragPtr.get()});

    auto vaoPtr = CreateVertexArray();
    vaoPtr->desc = VertexDesc;
    auto vboPtr = CreateBuffer(BufferType::Vertex, sizeof(vertices), vertices, BufferUsage::Static, 0);
    vaoPtr->ApplyLayout(vboPtr->id);

    programPtr->useShaderProgram();

    bool running = true;
    Uint64 lastFrame = SDL_GetPerformanceCounter();
    HideCursor();
    SetRelativeMouseMode(winPtr, true);

    // Camera
    Camera camera(90.0f, winPtr->width/winPtr->height, 0.1f, 10000.0f);

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

        programPtr->SetUniformMat4("u_View", camera.GetViewMatrix());
        programPtr->SetUniformMat4("u_Projection", camera.GetProjectionMatrix());

        glDrawArrays(GL_TRIANGLES, 0, 3);

        EndFrame();

        fps_counter();
    }

    // Cleanup
    DestroyWindow(winPtr);

    return 0;
}
