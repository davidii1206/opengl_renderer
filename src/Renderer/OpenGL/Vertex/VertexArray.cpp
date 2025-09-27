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
    glVertexArrayVertexBuffer(id, bindingIndex, vertexBufferID, offset, desc.stride);

    for (const auto& attrib : desc.attributes) {

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