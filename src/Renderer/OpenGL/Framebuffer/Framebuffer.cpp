#include "Framebuffer.h"
#include <iostream>

GLuint selectedFramebuffer;

float fboVertices[] = {
    -1.0f, -1.0f,  0.0f, 0.0f, 
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f, 
    -1.0f,  1.0f,  0.0f, 1.0f  
};

unsigned int fboIndices[] = {
    0, 1, 2,
    0, 2, 3  
};

Framebuffer::Framebuffer(ShaderProgram& program) : fboShaderProgram(program) {
    glCreateFramebuffers(1, &id);  
    vaoFBO = CreateVertexArray();
    vboFBO = CreateBuffer(BufferType::Vertex, sizeof(fboVertices), fboVertices);
    eboFBO = CreateBuffer(BufferType::Index, sizeof(fboIndices), fboIndices);
    vaoFBO->desc = FBOVertexDesc;
    vaoFBO->ApplyLayout(vboFBO->id);
    CreateFramebufferTex();
    program.useShaderProgram();
    program.SetUniform1i("screenTexture", 0);
}

Framebuffer::~Framebuffer() {
    if (rbo != 0) {
        glDeleteRenderbuffers(1, &rbo);
    }
    glDeleteFramebuffers(1, &id);
}

void Framebuffer::BindFramebuffer(GLuint fbo) {
    selectedFramebuffer = fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, s_CurrentWindow.width, s_CurrentWindow.height);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

void Framebuffer::UnbindFramebuffer(GLuint fbo) {
    if (selectedFramebuffer == fbo) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, s_CurrentWindow.width, s_CurrentWindow.height);
        DrawFramebuffer();
        selectedFramebuffer = 0;
    }
}

void Framebuffer::CreateFramebufferTex() {
    texFBO = CreateEmptyTexture(
        s_CurrentWindow.width, s_CurrentWindow.height,
        1, TextureType::Tex2D, TextureInternalFormat::RGBA8, 
        0, TextureFilter::Linear, TextureFilter::Linear,     
        TextureWrap::ClampToEdge, TextureWrap::ClampToEdge
    );
    glNamedFramebufferTexture(id, GL_COLOR_ATTACHMENT0, texFBO->id, 0);

    glCreateRenderbuffers(1, &rbo);
    glNamedRenderbufferStorage(rbo, GL_DEPTH_COMPONENT24, s_CurrentWindow.width, s_CurrentWindow.height);
    glNamedFramebufferRenderbuffer(id, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    GLenum status = glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FBO incomplete! Status: 0x" << std::hex << status << std::endl;
        
        // Print more detailed error information
        switch(status) {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" << std::endl;
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                std::cerr << "GL_FRAMEBUFFER_UNSUPPORTED" << std::endl;
                break;
            default:
                std::cerr << "Unknown framebuffer error" << std::endl;
                break;
        }
    } else {
        std::cout << "Framebuffer created successfully!" << std::endl;
    }
}

void Framebuffer::DrawFramebuffer() {
    fboShaderProgram.useShaderProgram();
    
    vaoFBO->bind();
    eboFBO->bind();  
    texFBO->BindTextureForSampling(0);

    glDisable(GL_DEPTH_TEST); 
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glEnable(GL_DEPTH_TEST);
}