#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <glad/glad.h>

enum class WindowMode { Windowed, Borderless, Fullscreen };

struct Window {
    uint32_t width;
    uint32_t height;
    std::string title;
    WindowMode mode;
    bool vsync;
    SDL_Window* handle = nullptr;
    SDL_GLContext context = nullptr;
    bool hasFocus;
};

struct OpenGLSettings {
    bool depthTest = true;
    GLenum depthFunc = GL_LESS;

    bool cullFace = true;
    GLenum cullMode = GL_BACK;

    bool blend = true;
    GLenum blendSrc = GL_SRC_ALPHA;
    GLenum blendDst = GL_ONE_MINUS_SRC_ALPHA;

    // Apply the settings to OpenGL
    void Apply() const {
        if (depthTest) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(depthFunc);
        } else {
            glDisable(GL_DEPTH_TEST);
        }

        if (cullFace) {
            glEnable(GL_CULL_FACE);
            glCullFace(cullMode);
        } else {
            glDisable(GL_CULL_FACE);
        }

        if (blend) {
            glEnable(GL_BLEND);
            glBlendFunc(blendSrc, blendDst);
        } else {
            glDisable(GL_BLEND);
        }
    }
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
std::vector<SDL_Event> PollEvents();

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