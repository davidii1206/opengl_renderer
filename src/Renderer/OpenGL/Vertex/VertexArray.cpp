#include "VertexArray.h"

VertexArray::VertexArray() {
    glGenVertexArrays(1, &id);
    glBindVertexArray(id);
}

VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &id);
    id = 0;
}

void VertexArray::ApplyLayout(GLuint vertexBufferID, GLuint bindingIndex, GLintptr offset) const {
    // Bind the buffer to the VAO at bindingIndex with the vertex stride
    glVertexArrayVertexBuffer(id, bindingIndex, vertexBufferID, offset, desc.stride);

    // Loop through each attribute and set it up
    for (const auto& attrib : desc.attributes) {
        // Enable the attribute in this VAO
        glEnableVertexArrayAttrib(id, attrib.index);


        glVertexArrayAttribFormat(
                id,
                attrib.index,
                attrib.componentCount,
                attrib.type,
                attrib.normalized,
                attrib.offset
            );

        glVertexArrayAttribBinding(id, attrib.index, bindingIndex);
    }
}