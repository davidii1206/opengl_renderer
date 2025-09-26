cmake_minimum_required(VERSION 4.0.2)

project(OpenGLRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(OpenGLRenderer 
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/Utils/stb_implementations.cpp
    ${PROJECT_SOURCE_DIR}/src/Renderer/OpenGL/Window/Window.cpp
    ${PROJECT_SOURCE_DIR}/src/Renderer/OpenGL/Buffer/Buffer.cpp
    ${PROJECT_SOURCE_DIR}/src/Renderer/OpenGL/Vertex/VertexArray.cpp
    ${PROJECT_SOURCE_DIR}/src/Renderer/OpenGL/Shader/Shader.cpp
    ${PROJECT_SOURCE_DIR}/src/Renderer/OpenGL/Shader/ShaderProgram.cpp
    ${PROJECT_SOURCE_DIR}/src/Renderer/OpenGL/Camera/Camera.cpp
    ${PROJECT_SOURCE_DIR}/src/Renderer/OpenGL/Texture/Texture.cpp
    ${PROJECT_SOURCE_DIR}/src/Renderer/OpenGL/Framebuffer/Framebuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/Renderer/OpenGL/Model/Model.cpp
    ${PROJECT_SOURCE_DIR}/src/Renderer/OpenGL/Mesh/Mesh.cpp
    ${PROJECT_SOURCE_DIR}/src/Renderer/OpenGL/Material/Material.cpp
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

# Add spdlog include folder
target_include_directories(OpenGLRenderer PRIVATE "${PROJECT_SOURCE_DIR}/lib")

if(MSVC)
    target_compile_options(OpenGLRenderer PRIVATE /utf-8)
endif()

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

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/src/Shader
  $<TARGET_FILE_DIR:${PROJECT_NAME}>/Shader
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/src/Textures
  $<TARGET_FILE_DIR:${PROJECT_NAME}>/Textures
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/src/Models
  $<TARGET_FILE_DIR:${PROJECT_NAME}>/Models
)