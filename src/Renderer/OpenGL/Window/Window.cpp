#include "Window.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <iostream>
#include <array>
#include "glad/glad.h"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>


Window* CreateWindow(Window& win) {
    // Initialize SDL (video subsystem)
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "[Window] SDL_Init Error: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    Uint32 flags = SDL_WINDOW_OPENGL;
    if (win.mode == WindowMode::Fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    } 
    else if (win.mode == WindowMode::Borderless) {
        flags |= SDL_WINDOW_BORDERLESS;
    }

    win.handle = SDL_CreateWindow(
        win.title.c_str(),
        win.width,
        win.height,
        flags
    );

    if (!win.handle) {
        std::cerr << "[Window] SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    win.context = SDL_GL_CreateContext(win.handle);
    if (!win.context) {
        std::cerr << "[Window] SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(win.handle);
        win.handle = nullptr;
        return nullptr;
    }

    SDL_GL_SetSwapInterval(win.vsync ? 1 : 0);

    win.hasFocus = true;

    s_CurrentWindow = win;

    return &win;
}

void DestroyWindow(Window* win) {
    if (!win) return;

    if (win->context) SDL_GL_DestroyContext(win->context);
    if (win->handle) SDL_DestroyWindow(win->handle);

    win->context = nullptr;
    win->handle = nullptr;

    SDL_Quit();
}

void SetWindowMode(Window* win, WindowMode mode) {
    if (!win) return;

    Uint32 flags = 0;
    switch (mode) {
        case WindowMode::Windowed:    flags = 0; break;
        case WindowMode::Borderless:  flags = SDL_WINDOW_BORDERLESS; break;
        case WindowMode::Fullscreen:  flags = SDL_WINDOW_FULLSCREEN; break;
    }

    SDL_SetWindowFullscreen(win->handle, flags);
    win->mode = mode;
}

void SetResolution(Window* win, int width, int height) {
    if (!win || !win->handle) return;

    SDL_SetWindowSize(win->handle, width, height);
    win->width = width;
    win->height = height;
}

void SetVsync(Window* win, bool enabled) {
    if (!win || !win->context) return;
    SDL_GL_SetSwapInterval(enabled ? 1 : 0);
    win->vsync = enabled;
}

void SwapBuffers(Window* win) {
    if (!win || !win->handle) return;
    SDL_GL_SwapWindow(win->handle);
}

std::vector<SDL_Event> PollEvents() {
    std::vector<SDL_Event> events;
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        events.push_back(event);
    }
    return events;
}

bool WindowHasFocus(const Window* win) {
    if (!win) return false;
    return win->hasFocus;
}

glm::ivec2 GetWindowSize(const Window* win) {
    if (!win) return glm::ivec2(0);
    return glm::ivec2(win->width, win->height);
}

glm::vec2 GetMousePos() {
    float x, y;
    
    SDL_GetMouseState(&x, &y);

    return glm::vec2{x, y};
}

void SetMousePos(Window* win, float x, float y) {
    SDL_WarpMouseInWindow(win->handle, x, y);
}

void HideCursor() {
    SDL_HideCursor();
}

void ShowCursor() {
    SDL_ShowCursor();
}

void SetRelativeMouseMode(Window* win, bool enabled) {
    SDL_SetWindowRelativeMouseMode(win->handle, enabled ? true : false);
}

void BeginFrame(const glm::vec4& clearColor) {
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void BeginFrameImGui(Window* win, const glm::vec4& clearColor) {
    glViewport(0, 0, win->width, win->height);

    glClearColor(clearColor.r, clearColor.b, clearColor.g, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //ImGui_ImplOpenGL3_NewFrame();
    //ImGui_ImplSDL3_NewFrame();
    //ImGui::NewFrame();
}

void EndFrame() {
    if (screenshotRequested == true) {
        TakeScreenshot();
        screenshotRequested = false;
    }

    SDL_GL_SwapWindow(s_CurrentWindow.handle);

    //rest to be added
}

void EndFrameImGui() {
    if (screenshotRequested == true) {
        TakeScreenshot();
        screenshotRequested = false;
    }

    SwapBuffers(&s_CurrentWindow);

    //ImGui::Render();
    //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    //rest to be added
}

void TakeScreenshot() {
    int width = s_CurrentWindow.width;
    int height = s_CurrentWindow.height;

    std::vector<unsigned char> pixels(width * height * 4);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    for (int y = 0; y < height / 2; ++y) {
        int top = y * width * 4;
        int bottom = (height - 1 - y) * width * 4;
        for (int x = 0; x < width * 4; ++x) {
            std::swap(pixels[top + x], pixels[bottom + x]);
        }
    }

    SDL_Surface* surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA32, pixels.data(), width * 4);


    if (!surface) {
        std::cerr << "Failed to create SDL_Surface: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_SaveBMP(surface, "Screenshot.bmp");
    SDL_DestroySurface(surface);
}

void AppendScreenshot() {
    screenshotRequested = true;
}