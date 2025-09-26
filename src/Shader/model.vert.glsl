#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;


layout(std140, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
};

uniform mat3 u_normalMatrix;
uniform mat4 u_modelMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec3 Tangent;
out vec3 Bitangent;
out mat3 TBN;

void main() {
    vec4 worldPos = u_modelMatrix * vec4(aPosition, 1.0);
    FragPos = worldPos.xyz;
    
    Normal = normalize(u_normalMatrix * aNormal);
    Tangent = normalize(u_normalMatrix * aTangent);
    Bitangent = normalize(u_normalMatrix * aBitangent);
    
    // Create TBN matrix for normal mapping
    TBN = mat3(Tangent, Bitangent, Normal);
    
    TexCoords = aTexCoords;
    
    gl_Position = projection * view * worldPos;
}