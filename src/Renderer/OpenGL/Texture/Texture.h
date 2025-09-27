#pragma once

#include <glad/glad.h>

#include <string>
#include <cstdint>
#include <vector>
#include <memory>

enum class TextureType : GLenum {
    Tex2D = GL_TEXTURE_2D,
    Tex3D = GL_TEXTURE_3D,
    CubeMap = GL_TEXTURE_CUBE_MAP
};

enum class TextureFormat : GLenum {
    RGB = GL_RGB,
    RGBA = GL_RGBA,
    RGBA8 = GL_RGBA8,
    Depth = GL_DEPTH_COMPONENT,
    DepthStencil = GL_DEPTH24_STENCIL8,
    RED = GL_RED
};

enum class TextureInternalFormat : GLenum {
    RGB8 = GL_RGB8,
    RGBA8 = GL_RGBA8,
    RGBA16F = GL_RGBA16F,
    RGB = GL_RGB,
    Depth24Stencil8 = GL_DEPTH24_STENCIL8,
    R8 = GL_R8
};

enum class TextureFilter : GLenum {
    Nearest = GL_NEAREST,
    Linear = GL_LINEAR,
    NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
    LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
    NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
    LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
};

enum class TextureWrap : GLenum {
    Repeat = GL_REPEAT,
    MirroredRepeat = GL_MIRRORED_REPEAT,
    ClampToEdge = GL_CLAMP_TO_EDGE,
    ClampToBorder = GL_CLAMP_TO_BORDER
};

enum class TextureAccess : GLenum {
    Read = GL_READ_ONLY,
    Write = GL_WRITE_ONLY,
    ReadWrite = GL_READ_WRITE
};

class Texture {
    public:
        Texture(
            const std::string& source,
            const std::vector<std::string>* faces = nullptr,
            TextureType type = TextureType::Tex2D,
            TextureFormat format = TextureFormat::RGBA8,
            TextureInternalFormat internalFormat = TextureInternalFormat::RGBA8,
            GLuint unit = 0,
            TextureFilter minFilter = TextureFilter::LinearMipmapLinear,
            TextureFilter magFilter = TextureFilter::Linear,
            TextureWrap wrapS = TextureWrap::Repeat,
            TextureWrap wrapT = TextureWrap::Repeat,
            TextureWrap wrapR = TextureWrap::Repeat
        );


        Texture(
            int width,
            int height,
            int depth,                
            TextureType type,
            TextureFormat format,
            TextureInternalFormat internalFormat,
            const void* data,
            GLuint unit = 0,
            TextureFilter minFilter = TextureFilter::Linear,
            TextureFilter magFilter = TextureFilter::Linear,
            TextureWrap wrapS = TextureWrap::Repeat,
            TextureWrap wrapT = TextureWrap::Repeat,
            TextureWrap wrapR = TextureWrap::Repeat
        );

        Texture(
            int width,
            int height,
            int depth,
            TextureType type,
            TextureInternalFormat internalFormat,
            GLuint unit = 0,
            TextureFilter minFilter = TextureFilter::Linear,
            TextureFilter magFilter = TextureFilter::Linear,
            TextureWrap wrapS = TextureWrap::Repeat,
            TextureWrap wrapT = TextureWrap::Repeat,
            TextureWrap wrapR = TextureWrap::Repeat
        );

        ~Texture();

        void BindTextureForSampling(GLuint unit) const { glBindTextureUnit(unit, id); }
        void BindTextureForImageAccess(GLuint binding, GLuint texture, TextureAccess access, TextureInternalFormat internalFormat) const { glBindImageTexture(binding, id, 0, GL_FALSE, 0, static_cast<GLenum>(access), static_cast<GLenum>(internalFormat)); }
        void BindTextureForImageAccess(GLuint binding, GLuint id, TextureAccess access, TextureInternalFormat internalFormat, 
                          int mipLevel, bool layered, int layer) const { glBindImageTexture(binding, id, mipLevel, layered, layer, static_cast<GLenum>(access), static_cast<GLenum>(internalFormat)); }
        
        void applyParameters();

    public:
        GLuint id = 0;
        int width = 0;
        int height = 0;
        int depth = 0;
        int nrChannels = 0;

        TextureFilter minFilter = TextureFilter::LinearMipmapLinear;
        TextureFilter magFilter = TextureFilter::Linear;
        TextureWrap wrapS = TextureWrap::Repeat;
        TextureWrap wrapT = TextureWrap::Repeat;
        TextureWrap wrapR = TextureWrap::Repeat;

        TextureType type;
        TextureFormat format;
        TextureInternalFormat internalFormat;
        const GLenum target;

    private: 
        void loadCubeMap(const std::vector<std::string>& faces);
};

inline std::unique_ptr<Texture> CreateTextureFromFile(
    const std::string& source,
    const std::vector<std::string>* faces = nullptr,
    TextureType type = TextureType::Tex2D,
    TextureFormat format = TextureFormat::RGBA,
    TextureInternalFormat internalFormat = TextureInternalFormat::RGBA8,
    GLuint unit = 0,
    TextureFilter minFilter = TextureFilter::LinearMipmapLinear,
    TextureFilter magFilter = TextureFilter::Linear,
    TextureWrap wrapS = TextureWrap::Repeat,
    TextureWrap wrapT = TextureWrap::Repeat,
    TextureWrap wrapR = TextureWrap::Repeat) 
{
    return std::make_unique<Texture>(source, faces, type, format, internalFormat, unit,
                                     minFilter, magFilter, wrapS, wrapT, wrapR);
}

inline std::unique_ptr<Texture> CreateTextureFromData(
    int width, int height, int depth,     
    TextureType type,
    TextureFormat format,
    TextureInternalFormat internalFormat,
    const void* data,
    GLuint unit = 0,
    TextureFilter minFilter = TextureFilter::Linear,
    TextureFilter magFilter = TextureFilter::Linear,
    TextureWrap wrapS = TextureWrap::Repeat,
    TextureWrap wrapT = TextureWrap::Repeat,
    TextureWrap wrapR = TextureWrap::Repeat)
{
    // Ensure depth is 1 for CubeMaps
    if (type == TextureType::CubeMap) depth = 1;

    return std::make_unique<Texture>(
        width, height, depth,
        type, format, internalFormat,
        data,
        unit, minFilter, magFilter, wrapS, wrapT, wrapR
    );
}

inline std::unique_ptr<Texture> CreateEmptyTexture(
    int width, int height, int depth,
    TextureType type,
    TextureInternalFormat internalFormat,
    GLuint unit = 0,
    TextureFilter minFilter = TextureFilter::Linear,
    TextureFilter magFilter = TextureFilter::Linear,
    TextureWrap wrapS = TextureWrap::Repeat,
    TextureWrap wrapT = TextureWrap::Repeat,
    TextureWrap wrapR = TextureWrap::Repeat) 
{
    return std::make_unique<Texture>(width, height, depth, type, internalFormat,
                                     unit, minFilter, magFilter, wrapS, wrapT, wrapR);
}

