#pragma once

#include <glad/glad.h>
#include <stb_image.h>
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
    DepthStencil = GL_DEPTH24_STENCIL8
};

enum class TextureInternalFormat : GLenum {
    RGB8 = GL_RGB8,
    RGBA8 = GL_RGBA8,
    RGBA16F = GL_RGBA16F,
    Depth24Stencil8 = GL_DEPTH24_STENCIL8
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

        ~Texture();

        void bind(GLuint unit) const { glBindTextureUnit(unit, id); }
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

inline std::unique_ptr<Texture> CreateTexture(            
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
            TextureWrap wrapR = TextureWrap::Repeat) {

    return std::make_unique<Texture>(source, faces, type, format, internalFormat, unit, minFilter, magFilter, wrapS, wrapT, wrapR);
}