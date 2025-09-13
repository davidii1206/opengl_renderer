#pragma once

#include "glm/glm.hpp"
#include <glad/glad.h>
#include <cstddef>
#include "VertexFormat.h"

struct CoreVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec4 color;
};

inline VertexDescription CoreVertexDesc{
    sizeof(CoreVertex),
    {
        {0, 3, GL_FLOAT, offsetof(CoreVertex, position), false},
        {1, 3, GL_FLOAT, offsetof(CoreVertex, normal), false},
        {2, 2, GL_FLOAT, offsetof(CoreVertex, texCoord), false},
        {3, 4, GL_FLOAT, offsetof(CoreVertex, color), false}
    }
};