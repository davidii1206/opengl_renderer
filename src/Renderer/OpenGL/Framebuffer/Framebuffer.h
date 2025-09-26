#pragma once

#include <memory>
#include <glad/glad.h>
#include "../Texture/Texture.h"
#include "../Window/Window.h"
#include "../Vertex/VertexArray.h"
#include "../Vertex/FBOVertex.h"
#include "../Buffer/Buffer.h"
#include "../Shader/ShaderProgram.h"

extern GLuint selectedFramebuffer;

extern float fboVertices[];
extern unsigned int fboIndices[];

class Framebuffer {
    public:
        Framebuffer(ShaderProgram& program);
        ~Framebuffer();

        void BindFramebuffer(GLuint fbo);
        void UnbindFramebuffer(GLuint fbo);

        GLuint id;
        GLuint rbo;
    private:
        void CreateFramebufferTex();
        void DrawFramebuffer();

        ShaderProgram& fboShaderProgram;  // Store reference to shader program
        std::unique_ptr<VertexArray> vaoFBO;
        std::unique_ptr<Buffer> vboFBO;
        std::unique_ptr<Buffer> eboFBO;
        std::unique_ptr<Texture> texFBO;
};

inline std::unique_ptr<Framebuffer> CreateFramebuffer(ShaderProgram& program) {
    return std::make_unique<Framebuffer>(program);
}