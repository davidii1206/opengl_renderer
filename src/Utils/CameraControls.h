#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_keyboard.h>
#include <glm/glm.hpp>
#include "../Renderer/OpenGL/Camera/Camera.h"
#include "../Renderer/OpenGL/Window/Window.h"

inline void mouse_callback(Window* win, Camera* camera, float xpos, float ypos) {
    static float sensitivity = 0.05f;
    static float lastX = win->width / 2;
    static float lastY = win->height / 2;
    static bool firstMouse = true;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    glm::vec2 rot = camera->GetRotation();
    rot.x += yoffset;
    rot.y += xoffset;

    if (rot.x > 89.0f) rot.x = 89.0f;
    if (rot.x < -89.0f) rot.x = -89.0f;

    camera->SetRotation(rot);
    camera->UpdateView();

    // Wrap the mouse to the center
    SDL_WarpMouseInWindow(win->handle, win->width / 2, win->height / 2);
    lastX = win->width / 2;
    lastY = win->height / 2;
}

inline void processInput(Window* window, Camera* camera, float deltaTime) {
    // Update the event queue
    SDL_PumpEvents();

    // Retrieve the current keyboard state
    const bool* keystate = SDL_GetKeyboardState(nullptr);

    // Define movement speed
    float speed = 50.f * deltaTime;

    // Adjust speed if the left control key is pressed
    if (keystate[SDL_SCANCODE_LCTRL]) {
        speed *= 4.0f; // 4 times faster
    }

    // Retrieve camera movement directions
    glm::vec3 forward = camera->GetForward();
    glm::vec3 right = camera->GetRight();
    glm::vec3 moveForward = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

    // Get the current camera position
    glm::vec3 pos = camera->GetPosition();

    // Update position based on key states
    if (keystate[SDL_SCANCODE_W]) pos += moveForward * speed;
    if (keystate[SDL_SCANCODE_S]) pos -= moveForward * speed;
    if (keystate[SDL_SCANCODE_A]) pos -= right * speed;
    if (keystate[SDL_SCANCODE_D]) pos += right * speed;
    if (keystate[SDL_SCANCODE_SPACE]) pos += worldUp * speed;
    if (keystate[SDL_SCANCODE_LSHIFT]) pos -= worldUp * speed;

    // Set the updated position to the camera
    camera->SetPosition(pos);
}

