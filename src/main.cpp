#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <glad/glad.h>
#include <iostream>
#include <memory>

#include "Renderer/OpenGL/Window/Window.h"
#include "Renderer/OpenGL/Buffer/Buffer.h"
#include "Renderer/OpenGL/Vertex/VertexArray.h"

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

    const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

    const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    auto vaoPtr = CreateVertexArray();
    vaoPtr->desc = VertexDesc;
    auto vboPtr = CreateBuffer(BufferType::Vertex, sizeof(vertices), vertices, BufferUsage::Static, 0);
    vaoPtr->ApplyLayout(vboPtr->id);
    glUseProgram(shaderProgram);

    // Main loop
    Uint64 lastTime = SDL_GetPerformanceCounter();
    Uint64 frameCount = 0;
    double elapsedTime = 0.0;
    double perfFreq = static_cast<double>(SDL_GetPerformanceFrequency());

    // Main loop
    bool running = true;
    while (running) {
        // Poll events
        auto events = PollEvents();
        for(auto &e : events) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_WINDOW_RESIZED) glViewport(0, 0, s_CurrentWindow.width, s_CurrentWindow.height);
        }

        BeginFrame(glm::vec4{0.1f, 0.1f, 0.1f, 1.f});

        glDrawArrays(GL_TRIANGLES, 0, 3);

        EndFrame();
    }

    // Cleanup
    DestroyWindow(winPtr);

    return 0;
}
