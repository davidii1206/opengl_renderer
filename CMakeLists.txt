cmake_minimum_required(VERSION 4.0.2)

project(OpenGLRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(OpenGLRenderer 
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/Renderer/OpenGL/Window/Window.cpp
    ${PROJECT_SOURCE_DIR}/lib/glad/src/glad.c
    ${PROJECT_SOURCE_DIR}/lib/imgui-1.92.2b/imgui.cpp
    ${PROJECT_SOURCE_DIR}/lib/imgui-1.92.2b/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/lib/imgui-1.92.2b/imgui_tables.cpp
    ${PROJECT_SOURCE_DIR}/lib/imgui-1.92.2b/imgui_widgets.cpp
    ${PROJECT_SOURCE_DIR}/lib/imgui-1.92.2b/backends/imgui_impl_sdl3.cpp
    ${PROJECT_SOURCE_DIR}/lib/imgui-1.92.2b/backends/imgui_impl_opengl3.cpp
)

set_source_files_properties(${PROJECT_SOURCE_DIR}/lib/glad/src/glad.c PROPERTIES LANGUAGE C)

#SDL
target_include_directories(OpenGLRenderer PRIVATE ${PROJECT_SOURCE_DIR}/lib/SDL3/include)

#ImGui
target_include_directories(OpenGLRenderer PRIVATE ${PROJECT_SOURCE_DIR}/lib/imgui-1.92.2b)
target_include_directories(OpenGLRenderer PRIVATE ${PROJECT_SOURCE_DIR}/lib/imgui-1.92.2b/backends)

#glad
target_include_directories(OpenGLRenderer PRIVATE ${PROJECT_SOURCE_DIR}/lib/glad/include)

target_include_directories(OpenGLRenderer PRIVATE ${PROJECT_SOURCE_DIR}/lib)

set(SDL3_LIBRARY ${PROJECT_SOURCE_DIR}/lib/SDL3/lib/x64/SDL3.lib)

target_link_libraries(OpenGLRenderer PRIVATE opengl32.lib ${SDL3_LIBRARY} shell32.lib)

if(MSVC)
    target_compile_options(OpenGLRenderer PRIVATE /FS)
endif()

add_custom_command(TARGET OpenGLRenderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/lib/SDL3/lib/x64/SDL3.dll"
        $<TARGET_FILE_DIR:OpenGLRenderer>)