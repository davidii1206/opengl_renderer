#pragma once
#include <glad/glad.h>
#include <cstdint>
#include <memory>
#include "VertexFormat.h"

class VertexArray {
    public:
        VertexArray();
        ~VertexArray();

       // Delete copy
        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;

        // Implement move
        VertexArray(VertexArray&& other) noexcept;
        VertexArray& operator=(VertexArray&& other) noexcept;

        void ApplyLayout(GLuint vertexBufferID, GLuint bindingIndex = 0, GLintptr offset = 0) const;
        void bind() const { glBindVertexArray(id); }

        GLuint id = 0;
        VertexDescription desc;
    private:
};

inline std::unique_ptr<VertexArray> CreateVertexArray() {
    return std::make_unique<VertexArray>();
}