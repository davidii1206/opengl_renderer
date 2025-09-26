#pragma once
#include <memory>
#include "glad/glad.h"

enum class BufferType { Vertex = 0, Index = 1, Uniform = 2, Storage = 3 };
enum class BufferUsage : GLenum { Static  = GL_STATIC_DRAW, Dynamic = GL_DYNAMIC_DRAW, Stream  = GL_STREAM_DRAW };
enum class BufferAccess : GLenum { Read = GL_READ_ONLY, Write = GL_WRITE_ONLY, Read_Write = GL_READ_WRITE };

class Buffer {
public:
    Buffer(BufferType type, size_t size, const void* data = nullptr, BufferUsage usage = BufferUsage::Static, GLuint bindingPoint = 0);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    void UpdateBuffer(const void* data, size_t size, size_t offset = 0);
    void* MapBuffer(GLenum access);
    void UnmapBuffer();
    void ReadBuffer(void* outData, size_t size, size_t offset = 0);
    void bind() { glBindBuffer(target, id); };

    GLuint id = 0;
    GLuint bindingPoint = 0;
    const GLenum target;
    size_t size = 0;

private:
    BufferType type;
};

inline std::unique_ptr<Buffer> CreateBuffer(BufferType type, size_t size, const void* data = nullptr, BufferUsage usage = BufferUsage::Static, GLuint bindingPoint = 0) {
    return std::make_unique<Buffer>(type, size, data, usage, bindingPoint);
}