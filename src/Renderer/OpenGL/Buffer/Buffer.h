#pragma once
#include <cstdint>

enum class BufferType { Vertex = 0, Index = 1, Uniform = 2, Storage = 3 };
enum class BufferUsage { Static = 0 , Dynamic = 1 , Stream = 2};

struct Buffer {
    uint32_t id;
    BufferType type;
    size_t size;
};

Buffer CreateBuffer(BufferType type, size_t size, const void* data = nullptr, BufferUsage usage = BufferUsage::Static);
void   UpdateBuffer(Buffer& buf, const void* data, size_t size, size_t offset = 0);
void*  MapBuffer(Buffer& buf);
void   UnmapBuffer(Buffer& buf);
void   DestroyBuffer(Buffer& buf);
void   ReadBuffer(Buffer& buf, void* outData, size_t size, size_t offset = 0);
