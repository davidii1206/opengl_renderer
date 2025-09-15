#pragma once

#include <cstdint>
#include <iostream>
#include <SDL3/SDL.h>

Uint64 lastTime = SDL_GetPerformanceCounter();
Uint64 frameCount = 0;
double elapsedTime = 0.0;
double perfFreq = static_cast<double>(SDL_GetPerformanceFrequency());

inline void fps_counter() {
    frameCount++;

    Uint64 now = SDL_GetPerformanceCounter();
    elapsedTime += static_cast<double>(now - lastTime) / perfFreq;
    lastTime = now;

    if (elapsedTime >= 1.0) { // 1 second elapsed
        std::cout << "FPS: " << frameCount << std::endl;
        frameCount = 0;
        elapsedTime = 0.0;
    }
}