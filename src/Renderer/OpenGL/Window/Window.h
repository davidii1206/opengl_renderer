#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <glad/glad.h>

enum class WindowMode { Windowed, Borderless, Fullscreen };

struct Window{
    uint32_t width;
    uint32_t height;
    std::string title;
    WindowMode mode;
    bool vsync;
    SDL_Window* handle = nullptr;
    SDL_GLContext context = nullptr;
    bool hasFocus;
};

inline Window window;

inline Window s_CurrentWindow;

inline bool screenshotRequested = false;

// TEMPORARY !!!!!!!!!!!!
struct RenderPass {
    GLuint framebuffer;
    glm::vec4 clearColor;
    bool clearColorBuffer;
    bool clearDepthBuffer;
};

// ------------------------
// Core functions
// ------------------------

Window* CreateWindow(Window& win);
void    DestroyWindow(Window* win);

void    SetWindowMode(Window* win, WindowMode mode);
void    SetResolution(Window* win, int width, int height);
void    SetVsync(Window* win, bool enabled);

void    SwapBuffers(Window* win);
void    PollEvents(Window* win);

bool    WindowHasFocus(const Window* win);
glm::ivec2 GetWindowSize(const Window* win);

// ------------------------------------------
// Input
// ------------------------------------------
glm::vec2 GetMousePos();
void      SetMousePos(Window* win, float x, float y);
void      HideCursor();
void      ShowCursor();
void      SetRelativeMouseMode(Window* win, bool enabled);

// ------------------------------------------
// Frame Lifecycle
// ------------------------------------------
void BeginFrame(const glm::vec4& clearColor = {0,0,0,1});
void EndFrame();

void BeginFrameImGui(Window* win, const glm::vec4& clearColor = {0,0,0,1});
void EndFrameImGui();

// Multiple framebuffer targets
void BeginRenderPass(RenderPass& fbo, const glm::vec4& clearColor);
void EndRenderPass();

// ------------------------------------------
// Utilities
// ------------------------------------------
void TakeScreenshot();
void AppendScreenshot();