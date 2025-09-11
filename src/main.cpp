#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <glad/glad.h>
#include <iostream>

#include "Renderer/OpenGL/Window/Window.h"

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

    // Set viewport
    glm::ivec2 size = GetWindowSize(winPtr);

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

    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;

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

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
        PollEvents(winPtr);

        // Check if window still has focus / not closed
        if (!WindowHasFocus(winPtr)) {
            running = false;
            break;
        }

        frameCount++;

        Uint64 now = SDL_GetPerformanceCounter();
        elapsedTime += static_cast<double>(now - lastTime) / perfFreq;
        lastTime = now;

        if (elapsedTime >= 1.0) { // 1 second elapsed
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            elapsedTime = 0.0;
        }
        BeginFrame(glm::vec4{0.1f, 0.1f, 0.1f, 1.f});

        glDrawArrays(GL_TRIANGLES, 0, 3);

        EndFrame();
    }

    // Cleanup
    DestroyWindow(winPtr);

    return 0;
}
