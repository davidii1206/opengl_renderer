#include "Texture.h"
#include <stdexcept>
#include <iostream>

#include <stb_image.h>

Texture::Texture(
    const std::string& source,
    const std::vector<std::string>* faces,
    TextureType type,
    TextureFormat format,
    TextureInternalFormat internalFormat,
    GLuint unit,
    TextureFilter minFilter,
    TextureFilter magFilter,
    TextureWrap wrapS,
    TextureWrap wrapT,
    TextureWrap wrapR
)
    : type(type),
      format(format),
      internalFormat(internalFormat),
      target(static_cast<GLenum>(type)),
      minFilter(minFilter),
      magFilter(magFilter),
      wrapS(wrapS),
      wrapT(wrapT),
      wrapR(wrapR)
{
    glCreateTextures(target, 1, &id);

    if (type == TextureType::CubeMap) {
        if (!faces) throw std::runtime_error("Cubemap faces vector cannot be null");
        
        // Use traditional glGenTextures for cubemaps for better compatibility
        glGenTextures(1, &id);
        loadCubeMap(*faces);
    } else {
        // Keep your existing 2D/3D texture code here
        glCreateTextures(target, 1, &id);
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(source.c_str(), &width, &height, &nrChannels, 0);
        if (!data) throw std::runtime_error("Failed to load texture: " + source);

        if (type == TextureType::Tex2D) {
            glTextureStorage2D(id, 1, static_cast<GLenum>(internalFormat), width, height);
            glTextureSubImage2D(id, 0, 0, 0, width, height,
                                static_cast<GLenum>(format), GL_UNSIGNED_BYTE, data);
        } else { // Tex3D
            depth = 1;
            glTextureStorage3D(id, 1, static_cast<GLenum>(internalFormat), width, height, depth);
            glTextureSubImage3D(id, 0, 0, 0, 0, width, height, depth,
                                static_cast<GLenum>(format), GL_UNSIGNED_BYTE, data);
        }

        stbi_image_free(data);
    }

    applyParameters();

    glBindTextureUnit(unit, id);

    if (minFilter == TextureFilter::LinearMipmapLinear) {
        glGenerateTextureMipmap(id);
    }
}

// From CPU data
Texture::Texture(
    int width,
    int height,
    int depth,
    TextureType type,
    TextureFormat format,
    TextureInternalFormat internalFormat,
    const void* data,
    GLuint unit,
    TextureFilter minFilter,
    TextureFilter magFilter,
    TextureWrap wrapS,
    TextureWrap wrapT,
    TextureWrap wrapR
)
    : width(width),
      height(height),
      depth(depth),
      type(type),
      format(format),
      internalFormat(internalFormat),
      target(static_cast<GLenum>(type)),
      minFilter(minFilter),
      magFilter(magFilter),
      wrapS(wrapS),
      wrapT(wrapT),
      wrapR(wrapR)
{
    glCreateTextures(target, 1, &id);

    if (type == TextureType::Tex2D) {
        glTextureStorage2D(id, 1, static_cast<GLenum>(internalFormat), width, height);
        glTextureSubImage2D(id, 0, 0, 0, width, height,
                            static_cast<GLenum>(format), GL_UNSIGNED_BYTE, data);
    } 
    else if (type == TextureType::Tex3D) {
        glTextureStorage3D(id, 1, static_cast<GLenum>(internalFormat), width, height, depth);
        glTextureSubImage3D(id, 0, 0, 0, 0, width, height, depth,
                            static_cast<GLenum>(format), GL_UNSIGNED_BYTE, data);
    } 
    else if (type == TextureType::CubeMap) {
        // Expecting 'data' to point to 6 * width * height * pixelSize bytes
        int pixelSize = (format == TextureFormat::RGBA || format == TextureFormat::RGBA8) ? 4 : 3;
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);

        for (int i = 0; i < 6; i++) {
            const void* faceData = static_cast<const unsigned char*>(data) + i * width * height * pixelSize;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                         static_cast<GLenum>(internalFormat), width, height, 0,
                         static_cast<GLenum>(format), GL_UNSIGNED_BYTE, faceData);
        }

        applyParameters();
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        return;
    }

    applyParameters();
    glBindTextureUnit(unit, id);
}

// Empty (for GPU writes / FBOs)
Texture::Texture(
    int width,
    int height,
    int depth,
    TextureType type,
    TextureInternalFormat internalFormat,
    GLuint unit,
    TextureFilter minFilter,
    TextureFilter magFilter,
    TextureWrap wrapS,
    TextureWrap wrapT,
    TextureWrap wrapR
)
    : width(width),
      height(height),
      depth(depth),
      type(type),
      format(TextureFormat::RGBA), // Default format (can refine later)
      internalFormat(internalFormat),
      target(static_cast<GLenum>(type)),
      minFilter(minFilter),
      magFilter(magFilter),
      wrapS(wrapS),
      wrapT(wrapT),
      wrapR(wrapR)
{
    glCreateTextures(target, 1, &id);
    std::cerr << "Created empty texture with ID: " << id << std::endl;

    if (type == TextureType::Tex2D) {
        glTextureStorage2D(id, 1, static_cast<GLenum>(internalFormat), width, height);
    } else if (type == TextureType::Tex3D) {
        glTextureStorage3D(id, 1, static_cast<GLenum>(internalFormat), width, height, depth);
    }

    applyParameters();
    glBindTextureUnit(unit, id);
}

Texture::~Texture() {
    if (id != 0) {
        glDeleteTextures(1, &id);
    }
}

void Texture::loadCubeMap(const std::vector<std::string>& faces) {
    if (faces.size() != 6)
        throw std::runtime_error("CubeMap requires 6 face textures!");

    // Bind the cubemap texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    // Don't flip vertically for cubemaps
    stbi_set_flip_vertically_on_load(false);

    for (size_t i = 0; i < faces.size(); i++) {
        int w, h, channels;
        unsigned char* data = stbi_load(faces[i].c_str(), &w, &h, &channels, 0);
        if (!data) {
            throw std::runtime_error("Failed to load CubeMap face: " + faces[i]);
        }

        // Store dimensions from first face
        if (i == 0) {
            width = w;
            height = h;
            nrChannels = channels;
        }

        // Load each face using the correct cubemap face target
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, // Face target (automatically increments)
            0,                                   // Mipmap level
            static_cast<GLenum>(internalFormat), // Internal format
            w, h,                               // Width, height
            0,                                  // Border (must be 0)
            static_cast<GLenum>(format),        // Format
            GL_UNSIGNED_BYTE,                   // Data type
            data                                // Pixel data
        );

        stbi_image_free(data);
    }

    // Apply texture parameters
    applyParameters();

    // Generate mipmaps if needed
    if (minFilter == TextureFilter::LinearMipmapLinear) {
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    // Unbind
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Texture::applyParameters() {
    if (type == TextureType::CubeMap) {
        // For cubemaps, bind first then set parameters
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        
        // Use linear filtering to reduce edge artifacts
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Clamp to edge to prevent sampling across faces
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    } else {
        // For regular textures, use DSA functions
        glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(minFilter));
        glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(magFilter));
        glTextureParameteri(id, GL_TEXTURE_WRAP_S, static_cast<GLenum>(wrapS));
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, static_cast<GLenum>(wrapT));

        if (type == TextureType::Tex3D)
            glTextureParameteri(id, GL_TEXTURE_WRAP_R, static_cast<GLenum>(wrapR));
    }
}