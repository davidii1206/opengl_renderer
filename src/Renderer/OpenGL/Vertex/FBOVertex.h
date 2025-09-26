#pragma once

#include "glm/glm.hpp"
#include <glad/glad.h>
#include <cstddef>
#include "VertexFormat.h"

struct FBOVertex {
    glm::vec2 position;
    glm::vec2 texCoord;
};

inline VertexDescription FBOVertexDesc{
    sizeof(FBOVertex),
    {
        {0, 2, GL_FLOAT, offsetof(FBOVertex, position), false},
        {1, 2, GL_FLOAT, offsetof(FBOVertex, texCoord), false},
    }
};