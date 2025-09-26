#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <glad/glad.h>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>

#include "Utils/fpscounter.h"
#include "Utils/CameraControls.h"
#include "Utils/Cubemap.h"
#include "Utils/Perlin.h"

#include "Renderer/OpenGL/Renderer.h"

int main(int argc, char* argv[]) {

    // Setup window parameters
    window.width  = 1920;
    window.height = 1080;
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
    Camera camera(110.0f, 16.0f/9.0f, 0.1f, 10000.0f);

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

    auto fboVert = CreateShader(ShaderStage::Vertex, "Shader/fbo.vert.glsl");
    auto fboFrag = CreateShader(ShaderStage::Fragment, "Shader/fbo.frag.glsl");
    auto fboProgram = CreateShaderProgram({fboVert.get(), fboFrag.get()});
    auto FBO = CreateFramebuffer(*fboProgram);

    auto UBOcamera = CreateBuffer(BufferType::Uniform, sizeof(glm::mat4) * 2, nullptr, BufferUsage::Dynamic, 0);
    
    bool running = true;
    Uint64 lastFrame = SDL_GetPerformanceCounter();
    HideCursor();
    SetRelativeMouseMode(winPtr, true);

    // Perlin
    initPerlin();

    auto model = CreateModel("Models/grabstein_2.glb");
    auto modelVertShader = CreateShader(ShaderStage::Vertex, "Shader/model.vert.glsl");
    auto modelFragShader = CreateShader(ShaderStage::Fragment, "Shader/model.frag.glsl");
    auto modelProgram = CreateShaderProgram({modelVertShader.get(), modelFragShader.get()});
    model->SetShaderForAllMaterials(std::shared_ptr<ShaderProgram>(modelProgram.release()));

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

        FBO->BindFramebuffer(FBO->id);

        // Update camera UBO
        UBOcamera->UpdateBuffer(glm::value_ptr(camera.GetViewMatrix()), sizeof(glm::mat4));
        UBOcamera->UpdateBuffer(glm::value_ptr(camera.GetProjectionMatrix()), sizeof(glm::mat4), sizeof(glm::mat4));

        // Draw triangle (optional - you can remove this)
        vaoPtr->bind();
        programPtr->useShaderProgram(); 
        glDrawArrays(GL_TRIANGLES, 0, 3);

        model->Draw();

        FBO->UnbindFramebuffer(FBO->id);

        EndFrame();

        fps_counter();
    }

    // Cleanup
    DestroyWindow(winPtr);

    return 0;
}