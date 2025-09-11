#include "Buffer.h"
#include <iostream>

Buffer CreateBuffer(BufferType type, size_t size, const void* data, BufferUsage usage = BufferUsage::Static) {
    switch (type) {
        case BufferType::Vertex:
        std::cout << "Vertex" << std::endl;
        break;
        case 1:
        std::cout << "Index" << std::endl;
        break;
        case 2:
        std::cout << "Uniform" << std::endl;
        break;
        case 3:
        std::cout << "Storage" << std::endl;
        break;
    }
}

void   UpdateBuffer(Buffer& buf, const void* data, size_t size, size_t offset);

void*  MapBuffer(Buffer& buf);

void   UnmapBuffer(Buffer& buf);

void   DestroyBuffer(Buffer& buf);

void   ReadBuffer(Buffer& buf, void* outData, size_t size, size_t offset);

