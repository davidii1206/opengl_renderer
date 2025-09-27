#include "Buffer.h"
#include <iostream>

Buffer::Buffer(BufferType type, size_t size, const void* data, BufferUsage usage, GLuint bindingPoint)
    : target(
        type == BufferType::Vertex  ? GL_ARRAY_BUFFER :
        type == BufferType::Index   ? GL_ELEMENT_ARRAY_BUFFER :
        type == BufferType::Uniform ? GL_UNIFORM_BUFFER :
        GL_SHADER_STORAGE_BUFFER
      )
{
    glCreateBuffers(1, &id);

    glNamedBufferData(id, size, data, static_cast<GLenum>(usage));

    if (type == BufferType::Storage || type == BufferType::Uniform) {
        glBindBufferBase(target, bindingPoint, id);
    }
}

Buffer::~Buffer() {
    if (id) glDeleteBuffers(1, &id);
    id = 0;
}

void Buffer::UpdateBuffer(const void* data, size_t size, size_t offset) {
    glNamedBufferSubData(id, offset, size, data);
}

void* Buffer::MapBuffer(GLenum access) {
    void *ptr = glMapNamedBuffer(id, access);
    return ptr;
}

void Buffer::UnmapBuffer() {
    glUnmapNamedBuffer(id);
}

void Buffer::ReadBuffer(void* outData, size_t size, size_t offset) {
    glGetNamedBufferSubData(id, offset, size, outData);
}