#pragma once
#include <cstdint>
#include <vector>

struct VertexAttrib {
    uint32_t index;          // Attribute location in shader
    int componentCount;      // How many components per attribute
    uint32_t type;           // Type of the attribute
    size_t offset;           // Byte offset
    bool normalized;         
};

struct VertexDescription {
    uint32_t stride;           // Byte stride between elements
    std::vector<VertexAttrib> attributes;
};